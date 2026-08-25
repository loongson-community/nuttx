/****************************************************************************
 * arch/loongarch/src/ls2k/ls_capture.c
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
#include "ls_capture.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* Configuration ************************************************************/

#if defined(GPIO_TIM1_CH1IN) || defined(GPIO_TIM2_CH1IN)
#  define HAVE_CH1IN 1
#endif

#if defined(GPIO_TIM1_CH2IN) || defined(GPIO_TIM2_CH2IN)
#  define HAVE_CH2IN 1
#endif

#if defined(GPIO_TIM1_CH3IN) || defined(GPIO_TIM2_CH3IN)
#  define HAVE_CH3IN 1
#endif

#if defined(GPIO_TIM1_CH4IN) || defined(GPIO_TIM2_CH4IN)
#  define HAVE_CH4IN 1
#endif

#ifdef CONFIG_LS_TIM1_CAP
#  define USE_ADVENCED_TIM 1
#endif

#if defined(GPIO_TIM1_EXT_CLK_IN) || defined(GPIO_TIM2_EXT_CLK_IN)
#  define USE_EXT_CLOCK 1
#endif

/* This module then only compiles if there are enabled timers that are not
 * intended for some other purpose.
 */

#if defined(CONFIG_LS_TIM1_CAP) || defined(CONFIG_LS_TIM2_CAP)

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* TIM Device Structure */

struct ls_cap_priv_s
{
  const struct ls_cap_ops_s *ops;
  const uintptr_t base;     /* TIMn base address */
#ifdef USE_EXT_CLOCK
  const uint32_t gpio_clk;  /* External clock input GPIO */
#endif
  const int irq;            /* irq vector */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/* Get a 32-bit register value by offset. */

static inline
uint32_t ls_getreg32(const struct ls_cap_priv_s *priv,
                     uint8_t offset)
{
  return getreg32(priv->base + offset);
}

/* Put a 32-bit register value by offset. */

static inline void ls_putreg32(const struct ls_cap_priv_s *priv,
                               uint8_t offset, uint32_t value)
{
  putreg32(value, priv->base + offset);
}

/* Modify a 32-bit register value by offset. */

static inline void ls_modifyreg32(const struct ls_cap_priv_s *priv,
                                  uint8_t offset, uint32_t clearbits,
                                  uint32_t setbits)
{
  modifyreg32(priv->base + offset, clearbits, setbits);
}

/****************************************************************************
 * gpio Functions
 ****************************************************************************/

static inline
uint32_t ls_cap_gpio(const struct ls_cap_priv_s *priv,
                     int channel)
{
  switch (priv->base)
    {
#ifdef CONFIG_LS_TIM1_CAP
      case LS_TIM1_BASE:
        switch (channel)
          {
#ifdef GPIO_TIM1_EXT_CLK_IN
            case LS_CAP_CHANNEL_COUNTER:
              return GPIO_TIM1_EXT_CLK_IN;
#endif
#ifdef GPIO_TIM1_CH1IN
            case 1:
              return GPIO_TIM1_CH1IN;
#endif
#ifdef GPIO_TIM1_CH2IN
            case 2:
              return GPIO_TIM1_CH2IN;
#endif
#ifdef GPIO_TIM1_CH3IN
            case 3:
              return GPIO_TIM1_CH3IN;
#endif
#ifdef GPIO_TIM1_CH4IN
            case 4:
              return GPIO_TIM1_CH4IN;
#endif
          }
        break;
#endif
#ifdef CONFIG_LS_TIM2_CAP
      case LS_TIM2_BASE:
        switch (channel)
          {
#ifdef GPIO_TIM2_EXT_CLK_IN
            case LS_CAP_CHANNEL_COUNTER:
              return GPIO_TIM2_EXT_CLK_IN;
#endif
#ifdef GPIO_TIM2_CH1IN
            case 1:
              return GPIO_TIM2_CH1IN;
#endif
#ifdef GPIO_TIM2_CH2IN
            case 2:
              return GPIO_TIM2_CH2IN;
#endif
#ifdef GPIO_TIM2_CH3IN
            case 3:
              return GPIO_TIM2_CH3IN;
#endif
#ifdef GPIO_TIM2_CH4IN
            case 4:
              return GPIO_TIM2_CH4IN;
#endif
          }
        break;
#endif
    }

  return 0;
}

/****************************************************************************
 * Basic Functions
 ****************************************************************************/

static int ls_cap_setclock(struct ls_cap_dev_s *dev,
                           uint32_t freq, uint32_t max)
{
  const struct ls_cap_priv_s *priv = (const struct ls_cap_priv_s *)dev;
  uint32_t freqin;
  int prescaler;

  /* Disable Timer? */

  if (freq == 0)
    {
      /* Disable Timer */

      ls_modifyreg32(priv, LS_BTIM_CR1_OFFSET, ATIM_CR1_CEN, 0);
      return 0;
    }

  /* Get the input clock frequency for this timer.  These vary with
   * different timer clock sources, MCU-specific timer configuration, and
   * board-specific clock configuration.  The correct input clock frequency
   * must be defined in the board.h header file.
   */

  freqin = LS_APB_FREQUENCY;

  /* Select a pre-scaler value for this timer using the input clock
   * frequency.
   */

  prescaler = freqin / freq;

  /* We need to decrement value for '1', but only, if we are allowed to
   * not to cause underflow. Check for overflow.
   */

  if (prescaler > 0)
    {
      prescaler--;
    }

  if (prescaler > 0xffff)
    {
      prescaler = 0xffff;
    }

  /* Set Maximum */

  ls_putreg32(priv, LS_BTIM_ARR_OFFSET, max);

  /* Set prescaler */

  ls_putreg32(priv, LS_BTIM_PSC_OFFSET, prescaler);

  /* Reset counter timer */

  ls_modifyreg32(priv, LS_BTIM_EGR_OFFSET, 0, BTIM_EGR_UG);

  /* Enable timer */

  ls_modifyreg32(priv, LS_BTIM_CR1_OFFSET, 0, BTIM_CR1_CEN);

#ifdef USE_ADVENCED_TIM
  /* Advanced registers require Main Output Enable */

  if (priv->base == LS_TIM1_BASE)
    {
      ls_modifyreg32(priv, LS_ATIM_BDTR_OFFSET, 0, ATIM_BDTR_MOE);
    }
#endif

  return prescaler;
}

/****************************************************************************
 * Name: ls_cap_setsmc
 *
 * Description:
 *   set slave mode control register
 *
 * Input Parameters:
 *   dev - A pointer of the ls capture device structure.
 *   cfg - Slave mode control register configure of timer.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int ls_cap_setsmc(struct ls_cap_dev_s *dev,
                         ls_cap_smc_cfg_t cfg)
{
  const struct ls_cap_priv_s *priv = (const struct ls_cap_priv_s *)dev;
  uint16_t regval = 0;
  uint16_t mask = 0;

  switch (cfg & LS_CAP_SMS_MASK)
    {
      case LS_CAP_SMS_INT:
          regval |= GTIM_SMCR_DISAB;
          break;

      case LS_CAP_SMS_ENC1:
          regval |= GTIM_SMCR_ENCMD1;
          break;

      case LS_CAP_SMS_ENC2:
          regval |= GTIM_SMCR_ENCMD2;
          break;

      case LS_CAP_SMS_ENC3:
          regval |= GTIM_SMCR_ENCMD3;
          break;

      case LS_CAP_SMS_RST:
          regval |= GTIM_SMCR_RESET;
          break;

      case LS_CAP_SMS_GAT:
          regval |= GTIM_SMCR_GATED;
          break;

      case LS_CAP_SMS_TRG:
          regval |= GTIM_SMCR_TRIGGER;
          break;

      case LS_CAP_SMS_EXT:
          regval |= GTIM_SMCR_EXTCLK1;
          break;

      default:
          break;
    }

  switch (cfg & LS_CAP_TS_MASK)
    {
      case LS_CAP_TS_ITR0:
        regval |= GTIM_SMCR_ITR0;
        break;

      case LS_CAP_TS_ITR1:
        regval |= GTIM_SMCR_ITR1;
        break;

      case LS_CAP_TS_ITR2:
        regval |= GTIM_SMCR_ITR2;
        break;

      case LS_CAP_TS_ITR3:
        regval |= GTIM_SMCR_ITR3;
        break;

      case LS_CAP_TS_TI1FED:
        regval |= GTIM_SMCR_TI1FED;
        break;

      case LS_CAP_TS_TI1FP1:
        regval |= GTIM_SMCR_TI1FP1;
        break;

      case LS_CAP_TS_TI2FP2:
        regval |= GTIM_SMCR_TI2FP2;
        break;

      case LS_CAP_TS_ETRF:
        regval |= GTIM_SMCR_ETRF;
        break;

      default:
        break;
    }

  if (cfg & LS_CAP_MSM_MASK)
    {
      regval |= LS_CAP_MSM_MASK;
    }

  mask = (LS_CAP_SMS_MASK | LS_CAP_TS_MASK | LS_CAP_MSM_MASK);
  ls_modifyreg32(priv, LS_GTIM_SMCR_OFFSET, mask, regval);

  return OK;
}

static int ls_cap_setisr(struct ls_cap_dev_s *dev, xcpt_t handler,
                         void *arg)
{
  const struct ls_cap_priv_s *priv = (const struct ls_cap_priv_s *)dev;
  int irq;

  DEBUGASSERT(dev != NULL);

  irq = priv->irq;

  /* Disable interrupt when callback is removed */

  if (!handler)
    {
      up_disable_irq(irq);
      irq_detach(irq);

      return OK;
    }

  /* Otherwise set callback and enable interrupt */

  irq_attach(irq, handler, arg);
  up_enable_irq(irq);

  return OK;
}

static void ls_cap_enableint(struct ls_cap_dev_s *dev,
                             ls_cap_flags_t src, bool on)
{
  const struct ls_cap_priv_s *priv = (const struct ls_cap_priv_s *)dev;
  uint16_t mask = 0;

  DEBUGASSERT(dev != NULL);

  if (src & LS_CAP_FLAG_IRQ_COUNTER)
    {
      mask |= ATIM_DIER_UIE;
    }

  if (src & LS_CAP_FLAG_IRQ_CH_1)
    {
      mask |= ATIM_DIER_CC1IE;
    }

  if (src & LS_CAP_FLAG_IRQ_CH_2)
    {
      mask |= ATIM_DIER_CC2IE;
    }

  if (src & LS_CAP_FLAG_IRQ_CH_3)
    {
      mask |= ATIM_DIER_CC3IE;
    }

  if (src & LS_CAP_FLAG_IRQ_CH_4)
    {
      mask |= ATIM_DIER_CC4IE;
    }

  /* Not IRQ on channel overflow */

  if (on)
    {
      ls_modifyreg32(priv, LS_BTIM_DIER_OFFSET, 0, mask);
    }
  else
    {
      ls_modifyreg32(priv, LS_BTIM_DIER_OFFSET, mask, 0);
    }
}

static void ls_cap_ackflags(struct ls_cap_dev_s *dev, int flags)
{
  const struct ls_cap_priv_s *priv = (const struct ls_cap_priv_s *)dev;
  uint16_t mask = 0;

  if (flags & LS_CAP_FLAG_IRQ_COUNTER)
    {
      mask |= ATIM_SR_UIF;
    }

  if (flags & LS_CAP_FLAG_IRQ_CH_1)
    {
      mask |= ATIM_SR_CC1IF;
    }

  if (flags & LS_CAP_FLAG_IRQ_CH_2)
    {
      mask |= ATIM_SR_CC2IF;
    }

  if (flags & LS_CAP_FLAG_IRQ_CH_3)
    {
      mask |= ATIM_SR_CC3IF;
    }

  if (flags & LS_CAP_FLAG_IRQ_CH_4)
    {
      mask |= ATIM_SR_CC4IF;
    }

  if (flags & LS_CAP_FLAG_OF_CH_1)
    {
      mask |= ATIM_SR_CC1OF;
    }

  if (flags & LS_CAP_FLAG_OF_CH_2)
    {
      mask |= ATIM_SR_CC2OF;
    }

  if (flags & LS_CAP_FLAG_OF_CH_3)
    {
      mask |= ATIM_SR_CC3OF;
    }

  if (flags & LS_CAP_FLAG_OF_CH_4)
    {
      mask |= ATIM_SR_CC4OF;
    }

  ls_putreg32(priv, LS_BTIM_SR_OFFSET, ~mask);
}

static ls_cap_flags_t ls_cap_getflags(struct ls_cap_dev_s *dev)
{
  const struct ls_cap_priv_s *priv = (const struct ls_cap_priv_s *)dev;
  uint16_t regval = 0;
  ls_cap_flags_t flags = 0;

  regval = ls_getreg32(priv, LS_BTIM_SR_OFFSET);

  if (regval & ATIM_SR_UIF)
    {
      flags |= LS_CAP_FLAG_IRQ_COUNTER;
    }

  if (regval & ATIM_SR_CC1IF)
    {
      flags |= LS_CAP_FLAG_IRQ_CH_1;
    }

  if (regval & ATIM_SR_CC2IF)
    {
      flags |= LS_CAP_FLAG_IRQ_CH_2;
    }

  if (regval & ATIM_SR_CC3IF)
    {
      flags |= LS_CAP_FLAG_IRQ_CH_3;
    }

  if (regval & ATIM_SR_CC4IF)
    {
      flags |= LS_CAP_FLAG_IRQ_CH_4;
    }

  if (regval & ATIM_SR_CC1OF)
    {
      flags |= LS_CAP_FLAG_OF_CH_1;
    }

  if (regval & ATIM_SR_CC2OF)
    {
      flags |= LS_CAP_FLAG_OF_CH_2;
    }

  if (regval & ATIM_SR_CC3OF)
    {
      flags |= LS_CAP_FLAG_OF_CH_3;
    }

  if (regval & ATIM_SR_CC4OF)
    {
      flags |= LS_CAP_FLAG_OF_CH_4;
    }

  return flags;
}

/****************************************************************************
 * General Functions
 ****************************************************************************/

static int ls_cap_setchannel(struct ls_cap_dev_s *dev,
                             uint8_t channel,
                             ls_cap_ch_cfg_t cfg)
{
  const struct ls_cap_priv_s *priv = (const struct ls_cap_priv_s *)dev;
  uint32_t gpio = 0;
  uint16_t mask;
  uint16_t regval;
  uint16_t ccer_en_bit;

  DEBUGASSERT(dev != NULL);

  gpio = ls_cap_gpio(priv, channel);

  if (gpio == 0)
    {
      return ERROR;
    }

  if ((cfg & LS_CAP_MAPPED_MASK) == 0)
    {
      return ERROR; /* MAPPED not selected */
    }

  /* Change to zero base index */

  channel--;

  /* Set ccer :
   *
   * GTIM_CCER_CCxE Is written latter to allow writing CCxS bits.
   *
   */

  switch (cfg & LS_CAP_EDGE_MASK)
    {
      case LS_CAP_EDGE_DISABLED:
        ccer_en_bit = 0;
        regval = 0;
        break;

      case LS_CAP_EDGE_RISING:
        ccer_en_bit = GTIM_CCER_CC1E;
        regval      = 0;
        break;

      case LS_CAP_EDGE_FALLING:
        ccer_en_bit = GTIM_CCER_CC1E;
        regval      = GTIM_CCER_CC1P;
        break;

      case LS_CAP_EDGE_BOTH:
        ccer_en_bit = GTIM_CCER_CC1E;
#ifdef HAVE_GTIM_CCXNP
        regval      = GTIM_CCER_CC1P | GTIM_CCER_CC1NP;
#else
        regval      = GTIM_CCER_CC1P;
#endif
        break;

      default:
        return ERROR;
    }

  /* Shift all CCER bits to corresponding channel */
#ifdef HAVE_GTIM_CCXNP
  mask = (GTIM_CCER_CC1E | GTIM_CCER_CC1P | GTIM_CCER_CC1NP);
#else
  mask = (GTIM_CCER_CC1E | GTIM_CCER_CC1P);
#endif
  mask          <<= GTIM_CCER_CCXBASE(channel);
  regval        <<= GTIM_CCER_CCXBASE(channel);
  ccer_en_bit   <<= GTIM_CCER_CCXBASE(channel);

  ls_modifyreg32(priv, LS_GTIM_CCER_OFFSET, mask, regval);

  /* Set ccmr */

  regval = cfg;
  mask = (GTIM_CCMR1_IC1F_MASK |
          GTIM_CCMR1_IC1PSC_MASK |
          GTIM_CCMR1_CC1S_MASK);
  regval &= mask;

  if (channel & 1)
    {
      regval <<= 8;
      mask   <<= 8;
    }

  if (channel < 2)
    {
      ls_modifyreg32(priv, LS_GTIM_CCMR1_OFFSET, mask, regval);
    }
  else
    {
      ls_modifyreg32(priv, LS_GTIM_CCMR2_OFFSET, mask, regval);
    }

  /* Set GPIO */

  if ((cfg & LS_CAP_EDGE_MASK) == LS_CAP_EDGE_DISABLED)
    {
      ls_unconfiggpio(gpio);
    }
  else
    {
      ls_configgpio(gpio);
    }

  /* Enable this channel timer */

  ls_modifyreg32(priv, LS_GTIM_CCER_OFFSET, 0, ccer_en_bit);
  return OK;
}

static uint32_t ls_cap_getcapture(struct ls_cap_dev_s *dev,
                                  uint8_t channel)
{
  const struct ls_cap_priv_s *priv = (const struct ls_cap_priv_s *)dev;
  uint32_t offset;

  DEBUGASSERT(dev != NULL);

  switch (channel)
    {
      case LS_CAP_CHANNEL_COUNTER:
        offset = LS_GTIM_CNT_OFFSET;
        break;
#ifdef HAVE_CH1IN
      case 1:
        offset = LS_GTIM_CCR1_OFFSET;
        break;
#endif
#ifdef HAVE_CH2IN
      case 2:
        offset = LS_GTIM_CCR2_OFFSET;
        break;
#endif
#ifdef HAVE_CH3IN
      case 3:
        offset = LS_GTIM_CCR3_OFFSET;
        break;
#endif
#ifdef HAVE_CH4IN
      case 4:
        offset = LS_GTIM_CCR4_OFFSET;
        break;
#endif
      default:
        return ERROR;
    }

  return ls_getreg32(priv, offset);
}

static uint32_t ls_cap_rstcounter(struct ls_cap_dev_s *dev)
{
  const struct ls_cap_priv_s *priv = (const struct ls_cap_priv_s *)dev;

  ls_modifyreg32(priv, LS_BTIM_EGR_OFFSET, 0, BTIM_EGR_UG);
  return OK;
}

/****************************************************************************
 * Advanced Functions
 ****************************************************************************/

/* TODO: Advanced functions for the LS_ATIM */

/****************************************************************************
 * Device Structures, Instantiation
 ****************************************************************************/

struct ls_cap_ops_s ls_cap_ops =
{
  .setsmc       = &ls_cap_setsmc,
  .setclock     = &ls_cap_setclock,
  .setchannel   = &ls_cap_setchannel,
  .getcapture   = &ls_cap_getcapture,
  .setisr       = &ls_cap_setisr,
  .enableint    = &ls_cap_enableint,
  .ackflags     = &ls_cap_ackflags,
  .getflags     = &ls_cap_getflags,
  .rstcounter   = &ls_cap_rstcounter,
};

#ifdef CONFIG_LS_TIM1_CAP
const struct ls_cap_priv_s ls_tim1_priv =
{
  .ops          = &ls_cap_ops,
  .base         = LS_TIM1_BASE,
  .irq          = LS_IRQ_TIM1,
};
#endif

#ifdef CONFIG_LS_TIM2_CAP
const struct ls_cap_priv_s ls_tim2_priv =
{
  .ops          = &ls_cap_ops,
  .base         = LS_TIM2_BASE,
  .irq          = LS_IRQ_TIM2,
};
#endif

static inline const struct ls_cap_priv_s *ls_cap_get_priv(int timer)
{
  switch (timer)
    {
#ifdef CONFIG_LS_TIM1_CAP
      case 1:
        return &ls_tim1_priv;
#endif
#ifdef CONFIG_LS_TIM2_CAP
      case 2:
        return &ls_tim2_priv;
#endif
    }

  return NULL;
}

/****************************************************************************
 * Public Function - Initialization
 ****************************************************************************/

struct ls_cap_dev_s *ls_cap_init(int timer)
{
  const struct ls_cap_priv_s *priv = ls_cap_get_priv(timer);
  uint32_t gpio;

  if (priv)
    {
      gpio = ls_cap_gpio(priv, LS_CAP_CHANNEL_COUNTER);
      if (gpio)
        {
          ls_configgpio(gpio);
        }

      /* Disable timer while is not configured */

      ls_modifyreg32(priv, LS_BTIM_CR1_OFFSET, ATIM_CR1_CEN, 0);
    }

  return (struct ls_cap_dev_s *)priv;
}

int ls_cap_deinit(struct ls_cap_dev_s *dev)
{
  const struct ls_cap_priv_s *priv = (struct ls_cap_priv_s *)dev;
  uint32_t gpio;

  DEBUGASSERT(dev != NULL);

  /* Disable timer while is not configured */

  ls_modifyreg32(priv, LS_BTIM_CR1_OFFSET, ATIM_CR1_CEN, 0);

  gpio = ls_cap_gpio(priv, LS_CAP_CHANNEL_COUNTER);
  if (gpio)
    {
      ls_unconfiggpio(gpio);
    }

  return OK;
}

#endif /* CONFIG_LS_TIM1_CAP || CONFIG_LS_TIM2_CAP */
