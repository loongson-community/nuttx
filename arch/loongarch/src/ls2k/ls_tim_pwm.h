/****************************************************************************
 * arch/loongarch/src/ls2k/ls_tim_pwm.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_TIM_PWM_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_TIM_PWM_H

/* The LS2K has a dedicated PWM controller.  In addition, its timers can be
 * used to generate pulsed outputs.  The logic in this file implements the
 * lower half of the standard NuttX PWM interface using the LS2K
 * timers.  That interface is described in include/nuttx/timers/pwm.h.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/timers/pwm.h>

#include "chip.h"
#include "hardware/ls_tim.h"

#ifdef CONFIG_LS_TIM_PWM
#  include <arch/board/board.h>
#  include "hardware/ls_tim.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* Timer devices may be used for different purposes.  One special purpose is
 * to generate modulated outputs for such things as motor control.
 * If CONFIG_LS_TIMn is defined then the CONFIG_LS_TIMn_PWM must also
 * be defined to indicate that timer "n" is intended to be used for pulsed
 * output signal generation.
 */

#ifndef CONFIG_LS_TIM1
#  undef CONFIG_LS_TIM1_PWM
#endif
#ifndef CONFIG_LS_TIM2
#  undef CONFIG_LS_TIM2_PWM
#endif

/* The basic timers (timer 6) are not capable of generating output pulses
 */

#undef CONFIG_LS_TIM6_PWM

/* Check if PWM support for any channel is enabled. */

#ifdef CONFIG_LS_TIM_PWM

/* PWM driver channels configuration */

#ifdef CONFIG_LS_TIM_PWM_MULTICHAN

#ifdef CONFIG_LS_TIM1_CHANNEL1
#  define PWM_TIM1_CHANNEL1 1
#else
#  define PWM_TIM1_CHANNEL1 0
#endif
#ifdef CONFIG_LS_TIM1_CHANNEL2
#  define PWM_TIM1_CHANNEL2 1
#else
#  define PWM_TIM1_CHANNEL2 0
#endif
#ifdef CONFIG_LS_TIM1_CHANNEL3
#  define PWM_TIM1_CHANNEL3 1
#else
#  define PWM_TIM1_CHANNEL3 0
#endif
#ifdef CONFIG_LS_TIM1_CHANNEL4
#  define PWM_TIM1_CHANNEL4 1
#else
#  define PWM_TIM1_CHANNEL4 0
#endif
#define PWM_TIM1_NCHANNELS (PWM_TIM1_CHANNEL1 + PWM_TIM1_CHANNEL2 + \
                            PWM_TIM1_CHANNEL3 + PWM_TIM1_CHANNEL4)

#ifdef CONFIG_LS_TIM2_CHANNEL1
#  define PWM_TIM2_CHANNEL1 1
#else
#  define PWM_TIM2_CHANNEL1 0
#endif
#ifdef CONFIG_LS_TIM2_CHANNEL2
#  define PWM_TIM2_CHANNEL2 1
#else
#  define PWM_TIM2_CHANNEL2 0
#endif
#ifdef CONFIG_LS_TIM2_CHANNEL3
#  define PWM_TIM2_CHANNEL3 1
#else
#  define PWM_TIM2_CHANNEL3 0
#endif
#ifdef CONFIG_LS_TIM2_CHANNEL4
#  define PWM_TIM2_CHANNEL4 1
#else
#  define PWM_TIM2_CHANNEL4 0
#endif
#define PWM_TIM2_NCHANNELS (PWM_TIM2_CHANNEL1 + PWM_TIM2_CHANNEL2 + \
                            PWM_TIM2_CHANNEL3 + PWM_TIM2_CHANNEL4)

#else  /* !CONFIG_LS_PWM_MULTICHAN */

/* For each timer that is enabled for PWM usage, we need the following
 * additional configuration settings:
 *
 * CONFIG_LS_TIMx_CHANNEL - Specifies the timer output channel {1,..,4}
 * PWM_TIMx_CHn - One of the values defined in chip/ls_pinmap.h.  In the
 * case where there are multiple pin selections, the correct setting must be
 * provided in the arch/board/board.h file.
 *
 * NOTE: The STM32 timers are each capable of generating different signals on
 * each of the four channels with different duty cycles.  That capability is
 * not supported by this driver:  Only one output channel per timer.
 */

#ifdef CONFIG_LS_TIM1_PWM
#  if !defined(CONFIG_LS_TIM1_CHANNEL)
#    error "CONFIG_LS_TIM1_CHANNEL must be provided"
#  elif CONFIG_LS_TIM1_CHANNEL == 1
#    define CONFIG_LS_TIM1_CHANNEL1 1
#    define CONFIG_LS_TIM1_CH1MODE  CONFIG_LS_TIM1_CHMODE
#  elif CONFIG_LS_TIM1_CHANNEL == 2
#    define CONFIG_LS_TIM1_CHANNEL2 1
#    define CONFIG_LS_TIM1_CH2MODE  CONFIG_LS_TIM1_CHMODE
#  elif CONFIG_LS_TIM1_CHANNEL == 3
#    define CONFIG_LS_TIM1_CHANNEL3 1
#    define CONFIG_LS_TIM1_CH3MODE  CONFIG_LS_TIM1_CHMODE
#  elif CONFIG_LS_TIM1_CHANNEL == 4
#    define CONFIG_LS_TIM1_CHANNEL4 1
#    define CONFIG_LS_TIM1_CH4MODE  CONFIG_LS_TIM1_CHMODE
#  else
#    error "Unsupported value of CONFIG_LS_TIM1_CHANNEL"
#  endif
#  define PWM_TIM1_NCHANNELS 1
#endif

#ifdef CONFIG_LS_TIM2_PWM
#  if !defined(CONFIG_LS_TIM2_CHANNEL)
#    error "CONFIG_LS_TIM2_CHANNEL must be provided"
#  elif CONFIG_LS_TIM2_CHANNEL == 1
#    define CONFIG_LS_TIM2_CHANNEL1 1
#    define CONFIG_LS_TIM2_CH1MODE  CONFIG_LS_TIM2_CHMODE
#  elif CONFIG_LS_TIM2_CHANNEL == 2
#    define CONFIG_LS_TIM2_CHANNEL2 1
#    define CONFIG_LS_TIM2_CH2MODE  CONFIG_LS_TIM2_CHMODE
#  elif CONFIG_LS_TIM2_CHANNEL == 3
#    define CONFIG_LS_TIM2_CHANNEL3 1
#    define CONFIG_LS_TIM2_CH3MODE  CONFIG_LS_TIM2_CHMODE
#  elif CONFIG_LS_TIM2_CHANNEL == 4
#    define CONFIG_LS_TIM2_CHANNEL4 1
#    define CONFIG_LS_TIM2_CH4MODE  CONFIG_LS_TIM2_CHMODE
#  else
#    error "Unsupported value of CONFIG_LS_TIM2_CHANNEL"
#  endif
#  define PWM_TIM2_NCHANNELS 1
#endif

#endif /* CONFIG_LS_PWM_MULTICHAN */

#ifdef CONFIG_LS_TIM1_CH1OUT
#  define PWM_TIM1_CH1CFG GPIO_TIM1_CH1OUT
#else
#  define PWM_TIM1_CH1CFG 0
#endif
#ifdef CONFIG_LS_TIM1_CH1NOUT
#  define PWM_TIM1_CH1NCFG GPIO_TIM1_CH1NOUT
#else
#  define PWM_TIM1_CH1NCFG 0
#endif
#ifdef CONFIG_LS_TIM1_CH2OUT
#  define PWM_TIM1_CH2CFG GPIO_TIM1_CH2OUT
#else
#  define PWM_TIM1_CH2CFG 0
#endif
#ifdef CONFIG_LS_TIM1_CH2NOUT
#  define PWM_TIM1_CH2NCFG GPIO_TIM1_CH2NOUT
#else
#  define PWM_TIM1_CH2NCFG 0
#endif
#ifdef CONFIG_LS_TIM1_CH3OUT
#  define PWM_TIM1_CH3CFG GPIO_TIM1_CH3OUT
#else
#  define PWM_TIM1_CH3CFG 0
#endif
#ifdef CONFIG_LS_TIM1_CH3NOUT
#  define PWM_TIM1_CH3NCFG GPIO_TIM1_CH3NOUT
#else
#  define PWM_TIM1_CH3NCFG 0
#endif
#ifdef CONFIG_LS_TIM1_CH4OUT
#  define PWM_TIM1_CH4CFG GPIO_TIM1_CH4OUT
#else
#  define PWM_TIM1_CH4CFG 0
#endif

#ifdef CONFIG_LS_TIM2_CH1OUT
#  define PWM_TIM2_CH1CFG GPIO_TIM2_CH1OUT
#else
#  define PWM_TIM2_CH1CFG 0
#endif
#ifdef CONFIG_LS_TIM2_CH2OUT
#  define PWM_TIM2_CH2CFG GPIO_TIM2_CH2OUT
#else
#  define PWM_TIM2_CH2CFG 0
#endif
#ifdef CONFIG_LS_TIM2_CH3OUT
#  define PWM_TIM2_CH3CFG GPIO_TIM2_CH3OUT
#else
#  define PWM_TIM2_CH3CFG 0
#endif
#ifdef CONFIG_LS_TIM2_CH4OUT
#  define PWM_TIM2_CH4CFG GPIO_TIM2_CH4OUT
#else
#  define PWM_TIM2_CH4CFG 0
#endif

/* Complementary outputs support */

#if defined(CONFIG_LS_TIM1_CH1NOUT) || defined(CONFIG_LS_TIM1_CH2NOUT) || \
    defined(CONFIG_LS_TIM1_CH3NOUT)
#  define HAVE_TIM1_COMPLEMENTARY
#endif
#if defined(HAVE_TIM1_COMPLEMENTARY)
#  define HAVE_PWM_COMPLEMENTARY
#endif

/* Low-level ops helpers ****************************************************/

#ifdef CONFIG_LS_TIM_PWM_LL_OPS

/* NOTE:
 * low-level ops accept pwm_lowerhalf_s as first argument, but llops access
 *       can be found in ls_pwm_dev_s
 */

#define PWM_SETUP(dev)                                                             \
        (dev)->ops->setup((struct pwm_lowerhalf_s *)dev)
#define PWM_SHUTDOWN(dev)                                                          \
        (dev)->ops->shutdown((struct pwm_lowerhalf_s *)dev)
#define PWM_CCR_UPDATE(dev, index, ccr)                                            \
        (dev)->llops->ccr_update((struct pwm_lowerhalf_s *)dev, index, ccr)
#define PWM_MODE_UPDATE(dev, index, mode)                                          \
        (dev)->llops->mode_update((struct pwm_lowerhalf_s *)dev, index, mode)
#define PWM_CCR_GET(dev, index)                                                    \
        (dev)->llops->ccr_get((struct pwm_lowerhalf_s *)dev, index)
#define PWM_ARR_UPDATE(dev, arr)                                                   \
        (dev)->llops->arr_update((struct pwm_lowerhalf_s *)dev, arr)
#define PWM_ARR_GET(dev)                                                           \
        (dev)->llops->arr_get((struct pwm_lowerhalf_s *)dev)
#define PWM_RCR_UPDATE(dev, rcr)                                                   \
        (dev)->llops->rcr_update((struct pwm_lowerhalf_s *)dev, rcr)
#define PWM_RCR_GET(dev)                                                           \
        (dev)->llops->rcr_get((struct pwm_lowerhalf_s *)dev)
#ifdef CONFIG_LS_TIM_PWM_TRGO
#  define PWM_TRGO_SET(dev, trgo)                                                  \
        (dev)->llops->trgo_set((struct pwm_lowerhalf_s *)dev, trgo)
#endif
#define PWM_OUTPUTS_ENABLE(dev, out, state)                                        \
        (dev)->llops->outputs_enable((struct pwm_lowerhalf_s *)dev, out, state)
#define PWM_SOFT_UPDATE(dev)                                                       \
        (dev)->llops->soft_update((struct pwm_lowerhalf_s *)dev)
#define PWM_CONFIGURE(dev)                                                         \
        (dev)->llops->configure((struct pwm_lowerhalf_s *)dev)
#define PWM_SOFT_BREAK(dev, state)                                                 \
        (dev)->llops->soft_break((struct pwm_lowerhalf_s *)dev, state)
#define PWM_FREQ_UPDATE(dev, freq)                                                 \
        (dev)->llops->freq_update((struct pwm_lowerhalf_s *)dev, freq)
#define PWM_TIM_ENABLE(dev, state)                                                 \
        (dev)->llops->tim_enable((struct pwm_lowerhalf_s *)dev, state)
#ifdef CONFIG_DEBUG_PWM_INFO
#  define PWM_DUMP_REGS(dev, msg)                                                  \
        (dev)->llops->dump_regs((struct pwm_lowerhalf_s *)dev, msg)
#else
#  define PWM_DUMP_REGS(dev, msg)
#endif
#define PWM_DT_UPDATE(dev, dt)                                                     \
        (dev)->llops->dt_update((struct pwm_lowerhalf_s *)dev, dt)
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Timer mode */

enum ls_pwm_tim_mode_e
{
  LS_TIMMODE_COUNTUP   = 0,
  LS_TIMMODE_COUNTDOWN = 1,
  LS_TIMMODE_CENTER1   = 2,
  LS_TIMMODE_CENTER2   = 3,
  LS_TIMMODE_CENTER3   = 4,
};

/* Timer output polarity */

enum ls_pwm_pol_e
{
  LS_POL_POS  = 0,
  LS_POL_NEG  = 1,
};

/* Timer output IDLE state */

enum ls_pwm_idle_e
{
  LS_IDLE_INACTIVE = 0,
  LS_IDLE_ACTIVE   = 1
};

/* PWM channel mode */

enum ls_pwm_chanmode_e
{
  LS_CHANMODE_FRZN        = 0,  /* CCRx matches has no effects on outputs */
  LS_CHANMODE_CHACT       = 1,  /* OCxREF active on match */
  LS_CHANMODE_CHINACT     = 2,  /* OCxREF inactive on match */
  LS_CHANMODE_OCREFTOG    = 3,  /* OCxREF toggles when TIMy_CNT=TIMyCCRx */
  LS_CHANMODE_OCREFLO     = 4,  /* OCxREF is forced low */
  LS_CHANMODE_OCREFHI     = 5,  /* OCxREF is forced high */
  LS_CHANMODE_PWM1        = 6,  /* PWM mode 1 */
  LS_CHANMODE_PWM2        = 7,  /* PWM mode 2 */
};

/* PWM timer channel */

enum ls_pwm_chan_e
{
  LS_PWM_CHAN1  = 1,
  LS_PWM_CHAN2  = 2,
  LS_PWM_CHAN3  = 3,
  LS_PWM_CHAN4  = 4,
};

/* PWM timer channel output */

enum ls_pwm_output_e
{
  LS_PWM_OUT1  = (1 << 0),
  LS_PWM_OUT1N = (1 << 1),
  LS_PWM_OUT2  = (1 << 2),
  LS_PWM_OUT2N = (1 << 3),
  LS_PWM_OUT3  = (1 << 4),
  LS_PWM_OUT3N = (1 << 5),
  LS_PWM_OUT4  = (1 << 6),

  /* 1 << 7 reserved - no complementary output for CH4 */
};

#ifdef CONFIG_LS_TIM_PWM_LL_OPS

/* This structure provides the publicly visible representation of the
 * "lower-half" PWM driver structure.
 */

struct ls_pwm_dev_s
{
  /* The first field of this state structure must be a pointer to the PWM
   * callback structure to be consistent with upper-half PWM driver.
   */

  const struct pwm_ops_s *ops;

  /* Publicly visible portion of the "lower-half" PWM driver structure */

  const struct ls_pwm_ops_s *llops;

  /* Require cast-compatibility with private "lower-half" PWM structure */
};

/* Low-level operations for PWM */

struct pwm_lowerhalf_s;
struct ls_pwm_ops_s
{
  /* Update CCR register */

  int (*ccr_update)(struct pwm_lowerhalf_s *dev,
                    uint8_t index, uint32_t ccr);

  /* Update PWM mode */

  int (*mode_update)(struct pwm_lowerhalf_s *dev,
                     uint8_t index, uint32_t mode);

  /* Get CCR register */

  uint32_t (*ccr_get)(struct pwm_lowerhalf_s *dev, uint8_t index);

  /* Update ARR register */

  int (*arr_update)(struct pwm_lowerhalf_s *dev, uint32_t arr);

  /* Get ARR register */

  uint32_t (*arr_get)(struct pwm_lowerhalf_s *dev);

  /* Update RCR register */

  int (*rcr_update)(struct pwm_lowerhalf_s *dev, uint16_t rcr);

  /* Get RCR register */

  uint16_t (*rcr_get)(struct pwm_lowerhalf_s *dev);

#ifdef CONFIG_LS_TIM_PWM_TRGO
  /* Set TRGO/TRGO2 register */

  int (*trgo_set)(struct pwm_lowerhalf_s *dev, uint8_t trgo);
#endif

  /* Enable outputs */

  int (*outputs_enable)(struct pwm_lowerhalf_s *dev, uint16_t outputs,
                        bool state);

  /* Software update */

  int (*soft_update)(struct pwm_lowerhalf_s *dev);

  /* PWM configure */

  int (*configure)(struct pwm_lowerhalf_s *dev);

  /* Software break */

  int (*soft_break)(struct pwm_lowerhalf_s *dev, bool state);

  /* Update frequency */

  int (*freq_update)(struct pwm_lowerhalf_s *dev, uint32_t frequency);

  /* Enable timer counter */

  int (*tim_enable)(struct pwm_lowerhalf_s *dev, bool state);

#ifdef CONFIG_DEBUG_PWM_INFO
  /* Dump timer registers */

  void (*dump_regs)(struct pwm_lowerhalf_s *dev, const char *msg);
#endif

#ifdef HAVE_PWM_COMPLEMENTARY
  /* Deadtime update */

  int (*dt_update)(struct pwm_lowerhalf_s *dev, uint8_t dt);
#endif
};

#endif /* CONFIG_LS_PWM_LL_OPS */

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: ls_pwminitialize
 *
 * Description:
 *   Initialize one timer for use with the upper_level PWM driver.
 *
 * Input Parameters:
 *   timer - A number identifying the timer use.  The number of valid timer
 *     IDs varies with the STM32 MCU and MCU family but is somewhere in
 *     the range of {1,..,17}.
 *
 * Returned Value:
 *   On success, a pointer to the STM32 lower half PWM driver is returned.
 *   NULL is returned on any failure.
 *
 ****************************************************************************/

struct pwm_lowerhalf_s *ls_pwminitialize(int timer);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* CONFIG_LS_TIM_PWM */
#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_TIM_PWM_H */
