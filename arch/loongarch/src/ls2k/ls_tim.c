/****************************************************************************
 * arch/loongarch/src/ls2k/ls_tim.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <nuttx/debug.h>

#include <arch/board/board.h>

#include "chip.h"
#include "loongarch_internal.h"
#include "ls.h"
#include "ls_gpio.h"
#include "ls_tim.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* Timer devices may be used for different purposes.  Such special purposes
 * include:
 *
 * - To generate modulated outputs for such things as motor control.  If
 *   CONFIG_LS_TIMn is defined then the CONFIG_LS_TIMn_PWM may also be
 *   defined to indicate that the timer is intended to be used for pulsed
 *   output modulation.
 *
 * - To control periodic ADC input sampling.  If CONFIG_LS_TIMn is
 *   defined then CONFIG_LS_TIMn_ADC may also be defined to indicate that
 *   timer "n" is intended to be used for that purpose.
 *
 * - To use a Quadrature Encoder.  If CONFIG_LS_TIMn is defined then
 *   CONFIG_LS_TIMn_QE may also be defined to indicate that timer "n" is
 *   intended to be used for that purpose.
 *
 * In any of these cases, the timer will not be used by this timer module.
 */

#if defined(CONFIG_LS_TIM1_PWM) || defined(CONFIG_LS_TIM1_ADC) || \
    defined(CONFIG_LS_TIM1_QE) || defined(CONFIG_LS_TIM1_CAP)
#  undef CONFIG_LS_TIM1
#endif
#if defined(CONFIG_LS_TIM2_PWM) || defined(CONFIG_LS_TIM2_ADC) || \
    defined(CONFIG_LS_TIM2_QE) ||  defined(CONFIG_LS_TIM2_CAP)
#  undef CONFIG_LS_TIM2
#endif
#if defined(CONFIG_LS_TIM6_PWM) || defined(CONFIG_LS_TIM6_ADC) || \
    defined(CONFIG_LS_TIM6_QE)
#  undef CONFIG_LS_TIM6
#endif

#undef HAVE_TIM_GPIOCONFIG
#if defined(CONFIG_LS_TIM1)
#  if defined(GPIO_TIM1_CH1OUT) ||defined(GPIO_TIM1_CH2OUT)||\
      defined(GPIO_TIM1_CH3OUT) ||defined(GPIO_TIM1_CH4OUT)
#    undef  HAVE_TIM_GPIOCONFIG
#    define HAVE_TIM_GPIOCONFIG  1
#    define HAVE_TIM1_GPIOCONFIG 1
#endif
#endif

#if defined(CONFIG_LS_TIM2)
#  if defined(GPIO_TIM2_CH1OUT) ||defined(GPIO_TIM2_CH2OUT)||\
      defined(GPIO_TIM2_CH3OUT) ||defined(GPIO_TIM2_CH4OUT)
#    undef  HAVE_TIM_GPIOCONFIG
#    define HAVE_TIM_GPIOCONFIG  1
#    define HAVE_TIM2_GPIOCONFIG 1
#endif
#endif

/* This module then only compiles if there are enabled timers that are not
 * intended for some other purpose.
 */

#if defined(CONFIG_LS_TIM1) || defined(CONFIG_LS_TIM2) || \
    defined(CONFIG_LS_TIM6)

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* TIM Device Structure */

struct ls_tim_priv_s
{
  const struct ls_tim_ops_s *ops;
  ls_tim_mode_t mode;
  uintptr_t base;                     /* TIMn base address */
};

/****************************************************************************
 * Private Function prototypes
 ****************************************************************************/

/* Register helpers */

static inline uint32_t ls_getreg32(struct ls_tim_dev_s *dev,
                                   uint8_t offset);
static inline void ls_putreg32(struct ls_tim_dev_s *dev,
                               uint8_t offset, uint32_t value);
static inline void ls_modifyreg32(struct ls_tim_dev_s *dev,
                                  uint8_t offset, uint32_t clearbits,
                                  uint32_t setbits);

/* Timer helpers */

static void ls_tim_reload_counter(struct ls_tim_dev_s *dev);
static void ls_tim_enable(struct ls_tim_dev_s *dev);
static void ls_tim_disable(struct ls_tim_dev_s *dev);
static void ls_tim_reset(struct ls_tim_dev_s *dev);

#ifdef HAVE_TIM_GPIOCONFIG
static void ls_tim_gpioconfig(uint32_t cfg, ls_tim_channel_t mode);
#endif

/* Timer methods */

static int  ls_tim_setmode(struct ls_tim_dev_s *dev,
                           ls_tim_mode_t mode);
static int  ls_tim_setclock(struct ls_tim_dev_s *dev,
                            uint32_t freq);
static void ls_tim_setperiod(struct ls_tim_dev_s *dev,
                             uint32_t period);
static uint32_t ls_tim_getcounter(struct ls_tim_dev_s *dev);
static void ls_tim_setcounter(struct ls_tim_dev_s *dev,
                              uint32_t count);
static int  ls_tim_getwidth(struct ls_tim_dev_s *dev);
static int  ls_tim_setchannel(struct ls_tim_dev_s *dev,
                              uint8_t channel, ls_tim_channel_t mode);
static int  ls_tim_setcompare(struct ls_tim_dev_s *dev,
                              uint8_t channel, uint32_t compare);
static int  ls_tim_getcapture(struct ls_tim_dev_s *dev,
                              uint8_t channel);
static int  ls_tim_setisr(struct ls_tim_dev_s *dev,
                          xcpt_t handler, void *arg, int source);
static void ls_tim_enableint(struct ls_tim_dev_s *dev,
                             int source);
static void ls_tim_disableint(struct ls_tim_dev_s *dev,
                              int source);
static void ls_tim_ackint(struct ls_tim_dev_s *dev, int source);
static int  ls_tim_checkint(struct ls_tim_dev_s *dev, int source);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct ls_tim_ops_s ls_tim_ops =
{
  .enable     = ls_tim_enable,
  .disable    = ls_tim_disable,
  .setmode    = ls_tim_setmode,
  .setclock   = ls_tim_setclock,
  .setperiod  = ls_tim_setperiod,
  .getcounter = ls_tim_getcounter,
  .setcounter = ls_tim_setcounter,
  .getwidth   = ls_tim_getwidth,
  .setchannel = ls_tim_setchannel,
  .setcompare = ls_tim_setcompare,
  .getcapture = ls_tim_getcapture,
  .setisr     = ls_tim_setisr,
  .enableint  = ls_tim_enableint,
  .disableint = ls_tim_disableint,
  .ackint     = ls_tim_ackint,
  .checkint   = ls_tim_checkint,
};

#ifdef CONFIG_LS_TIM1
struct ls_tim_priv_s ls_tim1_priv =
{
  .ops        = &ls_tim_ops,
  .mode       = LS_TIM_MODE_UNUSED,
  .base       = LS_TIM1_BASE,
};
#endif

#ifdef CONFIG_LS_TIM2
struct ls_tim_priv_s ls_tim2_priv =
{
  .ops        = &ls_tim_ops,
  .mode       = LS_TIM_MODE_UNUSED,
  .base       = LS_TIM2_BASE,
};
#endif

#ifdef CONFIG_LS_TIM6
struct ls_tim_priv_s ls_tim6_priv =
{
  .ops        = &ls_tim_ops,
  .mode       = LS_TIM_MODE_UNUSED,
  .base       = LS_TIM6_BASE,
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_getreg32
 *
 * Description:
 *   Get a 32-bit register value by offset
 *
 ****************************************************************************/

static inline uint32_t ls_getreg32(struct ls_tim_dev_s *dev,
                                   uint8_t offset)
{
  return getreg32(((struct ls_tim_priv_s *)dev)->base + offset);
}

/****************************************************************************
 * Name: ls_putreg32
 *
 * Description:
 *   Put a 32-bit register value by offset
 *
 ****************************************************************************/

static inline void ls_putreg32(struct ls_tim_dev_s *dev,
                               uint8_t offset, uint32_t value)
{
  putreg32(value, ((struct ls_tim_priv_s *)dev)->base + offset);
}

/****************************************************************************
 * Name: ls_modifyreg32
 *
 * Description:
 *   Modify a 32-bit register value by offset
 *
 ****************************************************************************/

static inline void ls_modifyreg32(struct ls_tim_dev_s *dev,
                                  uint8_t offset, uint32_t clearbits,
                                  uint32_t setbits)
{
  modifyreg32(((struct ls_tim_priv_s *)dev)->base + offset,
              clearbits, setbits);
}

/****************************************************************************
 * Name: ls_tim_reload_counter
 ****************************************************************************/

static void ls_tim_reload_counter(struct ls_tim_dev_s *dev)
{
  uint32_t val = ls_getreg32(dev, LS_GTIM_EGR_OFFSET);
  val |= GTIM_EGR_UG;
  ls_putreg32(dev, LS_GTIM_EGR_OFFSET, val);
}

/****************************************************************************
 * Name: ls_tim_enable
 ****************************************************************************/

static void ls_tim_enable(struct ls_tim_dev_s *dev)
{
  uint32_t val = ls_getreg32(dev, LS_GTIM_CR1_OFFSET);
  val |= GTIM_CR1_CEN;
  ls_tim_reload_counter(dev);
  ls_putreg32(dev, LS_GTIM_CR1_OFFSET, val);
}

/****************************************************************************
 * Name: ls_tim_disable
 ****************************************************************************/

static void ls_tim_disable(struct ls_tim_dev_s *dev)
{
  uint32_t val = ls_getreg32(dev, LS_GTIM_CR1_OFFSET);
  val &= ~GTIM_CR1_CEN;
  ls_putreg32(dev, LS_GTIM_CR1_OFFSET, val);
}

/****************************************************************************
 * Name: ls_tim_reset
 *
 * Description:
 *   Reset timer into system default state, but do not affect output/input
 *   pins
 *
 ****************************************************************************/

static void ls_tim_reset(struct ls_tim_dev_s *dev)
{
  ((struct ls_tim_priv_s *)dev)->mode = LS_TIM_MODE_DISABLED;
  ls_tim_disable(dev);
}

/****************************************************************************
 * Name: ls_tim_gpioconfig
 ****************************************************************************/

#ifdef HAVE_TIM_GPIOCONFIG
static void ls_tim_gpioconfig(uint32_t cfg, ls_tim_channel_t mode)
{
  /* TODO: Add support for input capture and bipolar dual outputs for TIM8 */

  if (mode & LS_TIM_CH_MODE_MASK)
    {
      ls_configgpio(cfg);
    }
  else
    {
      ls_unconfiggpio(cfg);
    }
}
#endif

/****************************************************************************
 * Name: ls_tim_setmode
 ****************************************************************************/

static int ls_tim_setmode(struct ls_tim_dev_s *dev,
                          ls_tim_mode_t mode)
{
  uint32_t val = GTIM_CR1_CEN | GTIM_CR1_ARPE;

  DEBUGASSERT(dev != NULL);

  /* This function is not supported on basic timers. To enable or
   * disable it, simply set its clock to valid frequency or zero.
   */

#if LS_NBTIM > 0
  if (((struct ls_tim_priv_s *)dev)->base == LS_TIM6_BASE)
    {
      return -EINVAL;
    }
#endif

  /* Decode operational modes */

  switch (mode & LS_TIM_MODE_MASK)
    {
      case LS_TIM_MODE_DISABLED:
        val = 0;
        break;

      case LS_TIM_MODE_DOWN:
        val |= GTIM_CR1_DIR;
        break;

      case LS_TIM_MODE_UP:
        break;

      case LS_TIM_MODE_UPDOWN:
        /* Our default:
         * Interrupts are generated on compare, when counting down
         */

        val |= GTIM_CR1_CENTER1;
        break;

      case LS_TIM_MODE_PULSE:
        val |= GTIM_CR1_OPM;
        break;

      default:
        return -EINVAL;
    }

  ls_tim_reload_counter(dev);
  ls_putreg32(dev, LS_GTIM_CR1_OFFSET, val);

#if LS_NATIM > 0
  /* Advanced registers require Main Output Enable */

  if (((struct ls_tim_priv_s *)dev)->base == LS_TIM1_BASE)
    {
      ls_modifyreg32(dev, LS_ATIM_BDTR_OFFSET, 0, ATIM_BDTR_MOE);
    }
#endif

  return OK;
}

/****************************************************************************
 * Name: ls_tim_setclock
 ****************************************************************************/

static int ls_tim_setclock(struct ls_tim_dev_s *dev, uint32_t freq)
{
  uint32_t freqin;
  int prescaler;

  DEBUGASSERT(dev != NULL);

  /* Disable Timer? */

  if (freq == 0)
    {
      ls_tim_disable(dev);
      return 0;
    }

  /* Get the input clock frequency for this timer. */

  switch (((struct ls_tim_priv_s *)dev)->base)
    {
#ifdef CONFIG_LS_TIM1
      case LS_TIM1_BASE:
        freqin = LS_APB_FREQUENCY;
        break;
#endif
#ifdef CONFIG_LS_TIM2
      case LS_TIM2_BASE:
        freqin = LS_APB_FREQUENCY;
        break;
#endif
#ifdef CONFIG_LS_TIM6
      case LS_TIM6_BASE:
        freqin = LS_APB_FREQUENCY;
        break;
#endif
      default:
        return -EINVAL;
    }

  /* Select a pre-scaler value for this timer using the input clock
   * frequency.
   */

  prescaler = freqin / freq;

  /* We need to decrement value for '1', but only, if that will not to
   * cause underflow.
   */

  if (prescaler > 0)
    {
      prescaler--;
    }

  /* Check for overflow as well. */

  if (prescaler > 0xffff)
    {
      prescaler = 0xffff;
    }

  ls_putreg32(dev, LS_GTIM_PSC_OFFSET, prescaler);
  ls_tim_enable(dev);

  return prescaler;
}

/****************************************************************************
 * Name: ls_tim_setperiod
 ****************************************************************************/

static void ls_tim_setperiod(struct ls_tim_dev_s *dev,
                             uint32_t period)
{
  DEBUGASSERT(dev != NULL);
  ls_putreg32(dev, LS_GTIM_ARR_OFFSET, period);
}

/****************************************************************************
 * Name: ls_tim_getcounter
 ****************************************************************************/

static uint32_t ls_tim_getcounter(struct ls_tim_dev_s *dev)
{
  DEBUGASSERT(dev != NULL);
  return ls_getreg32(dev, LS_GTIM_CNT_OFFSET);
}

/****************************************************************************
 * Name: ls_tim_setcounter
 ****************************************************************************/

static void ls_tim_setcounter(struct ls_tim_dev_s *dev,
                              uint32_t count)
{
  DEBUGASSERT(dev != NULL);
  ls_putreg32(dev, LS_GTIM_CNT_OFFSET, count);
}

/****************************************************************************
 * Name: ls_tim_getwidth
 ****************************************************************************/

static int ls_tim_getwidth(struct ls_tim_dev_s *dev)
{
  /* LS timers are 32-bit in width */

  (void)dev;
  return 32;
}

/****************************************************************************
 * Name: ls_tim_setchannel
 ****************************************************************************/

static int ls_tim_setchannel(struct ls_tim_dev_s *dev,
                             uint8_t channel, ls_tim_channel_t mode)
{
  uint32_t ccmr_orig   = 0;
  uint32_t ccmr_val    = 0;
  uint32_t ccmr_mask   = 0xff;
  uint32_t ccer_val    = ls_getreg32(dev, LS_GTIM_CCER_OFFSET);
  uint8_t  ccmr_offset = LS_GTIM_CCMR1_OFFSET;

  DEBUGASSERT(dev != NULL);

  /* Further we use range as 0..3; if channel=0 it will also overflow here */

  if (--channel > 4)
    {
      return -EINVAL;
    }

  /* Assume that channel is disabled and polarity is active high */

  ccer_val &= ~((GTIM_CCER_CC1P | GTIM_CCER_CC1E) <<
                GTIM_CCER_CCXBASE(channel));

  /* This function is not supported on basic timers. To enable or
   * disable it, simply set its clock to valid frequency or zero.
   */

#if LS_NBTIM > 0
  if (((struct ls_tim_priv_s *)dev)->base == LS_TIM6_BASE)
    {
      return -EINVAL;
    }
#endif

  /* Decode configuration */

  switch (mode & LS_TIM_CH_MODE_MASK)
    {
      case LS_TIM_CH_DISABLED:
        break;

      case LS_TIM_CH_OUTPWM:
        ccmr_val = (GTIM_CCMR_MODE_PWM1 << GTIM_CCMR1_OC1M_SHIFT) +
                   GTIM_CCMR1_OC1PE;
        ccer_val |= GTIM_CCER_CC1E << GTIM_CCER_CCXBASE(channel);
        break;

      default:
        return -EINVAL;
    }

  /* Set polarity */

  if (mode & LS_TIM_CH_POLARITY_NEG)
    {
      ccer_val |= GTIM_CCER_CC1P << GTIM_CCER_CCXBASE(channel);
    }

  /* Define its position (shift) and get register offset */

  if (channel & 1)
    {
      ccmr_val  <<= 8;
      ccmr_mask <<= 8;
    }

  if (channel > 1)
    {
      ccmr_offset = LS_GTIM_CCMR2_OFFSET;
    }

  ccmr_orig  = ls_getreg32(dev, ccmr_offset);
  ccmr_orig &= ~ccmr_mask;
  ccmr_orig |= ccmr_val;
  ls_putreg32(dev, ccmr_offset, ccmr_orig);
  ls_putreg32(dev, LS_GTIM_CCER_OFFSET, ccer_val);

  /* set GPIO */

  switch (((struct ls_tim_priv_s *)dev)->base)
    {
#ifdef CONFIG_LS_TIM1
      case LS_TIM1_BASE:
        switch (channel)
          {
#if defined(GPIO_TIM1_CH1OUT)
            case 0:
              ls_tim_gpioconfig(GPIO_TIM1_CH1OUT, mode); break;
#endif
#if defined(GPIO_TIM1_CH2OUT)
            case 1:
              ls_tim_gpioconfig(GPIO_TIM1_CH2OUT, mode); break;
#endif
#if defined(GPIO_TIM1_CH3OUT)
            case 2:
              ls_tim_gpioconfig(GPIO_TIM1_CH3OUT, mode); break;
#endif
#if defined(GPIO_TIM1_CH4OUT)
            case 3:
              ls_tim_gpioconfig(GPIO_TIM1_CH4OUT, mode); break;
#endif
            default:
              return -EINVAL;
          }
        break;
#endif
#ifdef CONFIG_LS_TIM2
      case LS_TIM2_BASE:
        switch (channel)
          {
#if defined(GPIO_TIM2_CH1OUT)
            case 0:
              ls_tim_gpioconfig(GPIO_TIM2_CH1OUT, mode);
              break;
#endif
#if defined(GPIO_TIM2_CH2OUT)
            case 1:
              ls_tim_gpioconfig(GPIO_TIM2_CH2OUT, mode);
              break;
#endif
#if defined(GPIO_TIM2_CH3OUT)
            case 2:
              ls_tim_gpioconfig(GPIO_TIM2_CH3OUT, mode);
              break;
#endif
#if defined(GPIO_TIM2_CH4OUT)
            case 3:
              ls_tim_gpioconfig(GPIO_TIM2_CH4OUT, mode);
              break;
#endif
            default:
              return -EINVAL;
          }
        break;
#endif
      default:
        return -EINVAL;
    }

  return OK;
}

/****************************************************************************
 * Name: ls_tim_setcompare
 ****************************************************************************/

static int ls_tim_setcompare(struct ls_tim_dev_s *dev,
                             uint8_t channel, uint32_t compare)
{
  DEBUGASSERT(dev != NULL);

  switch (channel)
    {
      case 1:
        ls_putreg32(dev, LS_GTIM_CCR1_OFFSET, compare);
        break;

      case 2:
        ls_putreg32(dev, LS_GTIM_CCR2_OFFSET, compare);
        break;

      case 3:
        ls_putreg32(dev, LS_GTIM_CCR3_OFFSET, compare);
        break;

      case 4:
        ls_putreg32(dev, LS_GTIM_CCR4_OFFSET, compare);
        break;

      default:
        return -EINVAL;
    }

  return OK;
}

/****************************************************************************
 * Name: ls_tim_getcapture
 ****************************************************************************/

static int ls_tim_getcapture(struct ls_tim_dev_s *dev,
                             uint8_t channel)
{
  DEBUGASSERT(dev != NULL);

  switch (channel)
    {
      case 1:
        return ls_getreg32(dev, LS_GTIM_CCR1_OFFSET);
      case 2:
        return ls_getreg32(dev, LS_GTIM_CCR2_OFFSET);
      case 3:
        return ls_getreg32(dev, LS_GTIM_CCR3_OFFSET);
      case 4:
        return ls_getreg32(dev, LS_GTIM_CCR4_OFFSET);
    }

  return -EINVAL;
}

/****************************************************************************
 * Name: ls_tim_setisr
 ****************************************************************************/

static int ls_tim_setisr(struct ls_tim_dev_s *dev, xcpt_t handler,
                         void * arg, int source)
{
  int vectorno;

  DEBUGASSERT(dev != NULL);
  DEBUGASSERT(source == 0);

  switch (((struct ls_tim_priv_s *)dev)->base)
    {
#ifdef CONFIG_LS_TIM1
      case LS_TIM1_BASE:
        vectorno = LS_IRQ_TIM1;
        break;
#endif
#ifdef CONFIG_LS_TIM2
      case LS_TIM2_BASE:
        vectorno = LS_IRQ_TIM2;
        break;
#endif
#ifdef CONFIG_LS_TIM6
      case LS_TIM6_BASE:
        vectorno = LS_IRQ_TIM6;
        break;
#endif
      default:
        return -EINVAL;
    }

  /* Disable interrupt when callback is removed */

  if (!handler)
    {
      up_disable_irq(vectorno);
      irq_detach(vectorno);
      return OK;
    }

  /* Otherwise set callback and enable interrupt */

  irq_attach(vectorno, handler, arg);
  up_enable_irq(vectorno);

  return OK;
}

/****************************************************************************
 * Name: ls_tim_enableint
 ****************************************************************************/

static void ls_tim_enableint(struct ls_tim_dev_s *dev, int source)
{
  DEBUGASSERT(dev != NULL);
  ls_modifyreg32(dev, LS_GTIM_DIER_OFFSET, 0, source);
}

/****************************************************************************
 * Name: ls_tim_disableint
 ****************************************************************************/

static void ls_tim_disableint(struct ls_tim_dev_s *dev, int source)
{
  DEBUGASSERT(dev != NULL);
  ls_modifyreg32(dev, LS_GTIM_DIER_OFFSET, source, 0);
}

/****************************************************************************
 * Name: ls_tim_ackint
 ****************************************************************************/

static void ls_tim_ackint(struct ls_tim_dev_s *dev, int source)
{
  ls_putreg32(dev, LS_GTIM_SR_OFFSET, ~source);
}

/****************************************************************************
 * Name: ls_tim_checkint
 ****************************************************************************/

static int ls_tim_checkint(struct ls_tim_dev_s *dev, int source)
{
  uint32_t regval = ls_getreg32(dev, LS_GTIM_SR_OFFSET);
  return (regval & source) ? 1 : 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_tim_init
 ****************************************************************************/

struct ls_tim_dev_s *ls_tim_init(int timer)
{
  struct ls_tim_dev_s *dev = NULL;

  /* Get structure */

  switch (timer)
    {
#ifdef CONFIG_LS_TIM1
      case 1:
        dev = (struct ls_tim_dev_s *)&ls_tim1_priv;
        break;
#endif
#ifdef CONFIG_LS_TIM2
      case 2:
        dev = (struct ls_tim_dev_s *)&ls_tim2_priv;
        break;
#endif
#ifdef CONFIG_LS_TIM6
      case 6:
        dev = (struct ls_tim_dev_s *)&ls_tim6_priv;
        break;
#endif
      default:
        return NULL;
    }

  /* Is device already allocated */

  if (((struct ls_tim_priv_s *)dev)->mode != LS_TIM_MODE_UNUSED)
    {
      return NULL;
    }

  ls_tim_reset(dev);

  return dev;
}

/****************************************************************************
 * Name: ls_tim_deinit
 *
 * TODO: Detach interrupts, and close down all TIM Channels
 *
 ****************************************************************************/

int ls_tim_deinit(struct ls_tim_dev_s * dev)
{
  DEBUGASSERT(dev != NULL);

  /* Mark it as free */

  ((struct ls_tim_priv_s *)dev)->mode = LS_TIM_MODE_UNUSED;

  return OK;
}

#endif /* defined(CONFIG_LS_TIM1 || TIM2 || TIM6) */
