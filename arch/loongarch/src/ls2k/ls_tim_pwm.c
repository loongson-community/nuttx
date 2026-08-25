/****************************************************************************
 * arch/loongarch/src/ls2k/ls_tim_pwm.c
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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <nuttx/debug.h>

#include <nuttx/arch.h>
#include <arch/board/board.h>

#include "loongarch_internal.h"
#include "chip.h"
#include "ls_tim_pwm.h"
#include "ls_gpio.h"

#ifdef CONFIG_LS_TIM_PWM

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* PWM/Timer Definitions ****************************************************/

/* The following definitions are used to identify the various time types.
 */

#define TIMTYPE_BASIC        0  /* Basic timers (no outputs) */
#define TIMTYPE_GENERAL32    4  /* General 32-bit timers (up, down, up/down)*/
#define TIMTYPE_ADVANCED     5  /* Advanced timers */

#define TIMTYPE_TIM1       TIMTYPE_ADVANCED
#define TIMTYPE_TIM2       TIMTYPE_GENERAL32
#define TIMTYPE_TIM6       TIMTYPE_BASIC

/* Timer clock source */
#define   TIMCLK_TIM1        LS_APB_FREQUENCY
#define   TIMCLK_TIM2        LS_APB_FREQUENCY
#define   TIMCLK_TIM6        LS_APB_FREQUENCY

/* Advanced Timer support */

#if defined(CONFIG_LS_TIM1_PWM)
#  define HAVE_ADVTIM
#else
#  undef HAVE_ADVTIM
#endif

/* TRGO support */

#define HAVE_TRGO

/* Break support */

#if defined(CONFIG_LS_TIM1_BREAK1)
#  defined HAVE_BREAK
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* PWM output configuration */

struct ls_pwm_out_s
{
  uint8_t  in_use:1;                    /* Output in use */
  uint8_t  pol:1;                       /* Polarity. Default: positive */
  uint8_t  idle:1;                      /* Idle state. Default: inactive */
  uint8_t  _res:5;                      /* Reserved */
  uint32_t pincfg;                      /* Output pin configuration */
};

/* PWM break configuration */

#ifdef HAVE_BREAK
struct ls_pwm_break_s
{
  uint8_t en1:1;                        /* Break 1 enable */
  uint8_t pol1:1;                       /* Break 1 polarity */
  uint8_t _res:6;                       /* Reserved */
};
#endif

/* PWM channel configuration */

struct ls_pwmchan_s
{
  uint8_t                  channel:4;   /* Timer output channel: {1,..4} */
  uint8_t                  mode:4;      /* PWM channel mode (see ls_pwm_chanmode_e) */
  struct ls_pwm_out_s      out1;        /* PWM output configuration */
#ifdef HAVE_BREAK
  struct ls_pwm_break_s    brk;         /* PWM break configuration */
#endif
#ifdef HAVE_PWM_COMPLEMENTARY
  struct ls_pwm_out_s      out2;        /* PWM complementary output configuration */
#endif
};

/* This structure represents the state of one PWM timer */

struct ls_pwmtimer_s
{
  const struct pwm_ops_s *ops;      /* PWM operations */
#ifdef CONFIG_LS_PWM_LL_OPS
  const struct ls_pwm_ops_s *llops; /* Low-level PWM ops */
#endif
  struct ls_pwmchan_s *channels;     /* Channels configuration */
  uint8_t   timid:5;                 /* Timer ID {1,...,17} */
  uint8_t   chan_num:3;              /* Number of configured channels */
  uint8_t   timtype:3;               /* See the TIMTYPE_* definitions */
  uint8_t   mode:3;                  /* Timer mode (see ls_pwm_tim_mode_e) */
  uint8_t   lock:2;                  /* Lock configuration */
  uint8_t   t_dts:3;                 /* Clock division for t_DTS */
  uint8_t   _res:5;                  /* Reserved */
#ifdef HAVE_PWM_COMPLEMENTARY
  uint8_t   deadtime;                /* Dead-time value */
#endif
#ifdef HAVE_TRGO
  uint8_t   trgo;                    /* TRGO configuration:
                                     * 4 LSB = TRGO, 4 MSB = TRGO2
                                     */
#endif
  uint32_t  frequency;               /* Current frequency setting */
  uintptr_t base;                    /* The base address of the timer */
  uint32_t  pclk;                    /* The frequency of the peripheral
                                     * clock that drives the timer module
                                     */
};

/****************************************************************************
 * Static Function Prototypes
 ****************************************************************************/

/* Register access */

static uint32_t pwm_getreg(struct ls_pwmtimer_s *priv, int offset);
static void pwm_putreg(struct ls_pwmtimer_s *priv, int offset,
                       uint32_t value);
static void pwm_modifyreg(struct ls_pwmtimer_s *priv, uint32_t offset,
                          uint32_t clearbits, uint32_t setbits);

#ifdef CONFIG_DEBUG_PWM_INFO
static void pwm_dumpregs(struct pwm_lowerhalf_s *dev,
                         const char *msg);
#else
#  define pwm_dumpregs(priv,msg)
#endif

/* Timer management */

static int pwm_frequency_update(struct pwm_lowerhalf_s *dev,
                                uint32_t frequency);
static int pwm_mode_configure(struct pwm_lowerhalf_s *dev,
                              uint8_t channel, uint32_t mode);
static int pwm_timer_configure(struct ls_pwmtimer_s *priv);
static int pwm_output_configure(struct ls_pwmtimer_s *priv,
                                struct ls_pwmchan_s *chan);
static int pwm_outputs_enable(struct pwm_lowerhalf_s *dev,
                              uint16_t outputs, bool state);
static int pwm_soft_update(struct pwm_lowerhalf_s *dev);
static int pwm_soft_break(struct pwm_lowerhalf_s *dev, bool state);
static int pwm_ccr_update(struct pwm_lowerhalf_s *dev, uint8_t index,
                          uint32_t ccr);
static int pwm_arr_update(struct pwm_lowerhalf_s *dev, uint32_t arr);
static uint32_t pwm_arr_get(struct pwm_lowerhalf_s *dev);
static int pwm_duty_update(struct pwm_lowerhalf_s *dev, uint8_t channel,
                           ub16_t duty);
static int pwm_timer_enable(struct pwm_lowerhalf_s *dev, bool state);

#ifdef HAVE_ADVTIM
static int pwm_break_dt_configure(struct ls_pwmtimer_s *priv);
#endif
#ifdef HAVE_TRGO
static int pwm_trgo_configure(struct pwm_lowerhalf_s *dev,
                              uint8_t trgo);
#endif
#if defined(HAVE_PWM_COMPLEMENTARY) && defined(CONFIG_LS_PWM_LL_OPS)
static int pwm_deadtime_update(struct pwm_lowerhalf_s *dev, uint8_t dt);
#endif
#ifdef CONFIG_LS_PWM_LL_OPS
static uint32_t pwm_ccr_get(struct pwm_lowerhalf_s *dev, uint8_t index);
static uint16_t pwm_rcr_get(struct pwm_lowerhalf_s *dev);
#endif
#ifdef HAVE_ADVTIM
static int pwm_rcr_update(struct pwm_lowerhalf_s *dev, uint16_t rcr);
#endif

static int pwm_configure(struct pwm_lowerhalf_s *dev);
static int pwm_timer(struct pwm_lowerhalf_s *dev,
                     const struct pwm_info_s *info);

/* PWM driver methods */

static int pwm_setup(struct pwm_lowerhalf_s *dev);
static int pwm_shutdown(struct pwm_lowerhalf_s *dev);

static int pwm_start(struct pwm_lowerhalf_s *dev,
                     const struct pwm_info_s *info);

static int pwm_stop(struct pwm_lowerhalf_s *dev);
static int pwm_ioctl(struct pwm_lowerhalf_s *dev,
                     int cmd, unsigned long arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* This is the list of lower half PWM driver methods used by the upper half
 * driver.
 */

static const struct pwm_ops_s g_pwmops =
{
  .setup       = pwm_setup,
  .shutdown    = pwm_shutdown,
  .start       = pwm_start,
  .stop        = pwm_stop,
  .ioctl       = pwm_ioctl,
};

#ifdef CONFIG_LS_PWM_LL_OPS
static const struct ls_pwm_ops_s g_llpwmops =
{
  .configure       = pwm_configure,
  .soft_break      = pwm_soft_break,
  .ccr_update      = pwm_ccr_update,
  .mode_update     = pwm_mode_configure,
  .ccr_get         = pwm_ccr_get,
  .arr_update      = pwm_arr_update,
  .arr_get         = pwm_arr_get,
#ifdef HAVE_ADVTIM
  .rcr_update      = pwm_rcr_update,
#endif
  .rcr_get         = pwm_rcr_get,
#ifdef HAVE_TRGO
  .trgo_set        = pwm_trgo_configure,
#endif
  .outputs_enable  = pwm_outputs_enable,
  .soft_update     = pwm_soft_update,
  .freq_update     = pwm_frequency_update,
  .tim_enable      = pwm_timer_enable,
#  ifdef CONFIG_DEBUG_PWM_INFO
  .dump_regs       = pwm_dumpregs,
#  endif
#  ifdef HAVE_PWM_COMPLEMENTARY
  .dt_update       = pwm_deadtime_update,
#  endif
};
#endif

#ifdef CONFIG_LS_TIM1_PWM

static struct ls_pwmchan_s g_pwm1channels[] =
{
  /* TIM1 has 4 channels, 4 complementary */

#ifdef CONFIG_LS_TIM1_CHANNEL1
  {
    .channel = 1,
    .mode    = CONFIG_LS_TIM1_CH1MODE,
#ifdef HAVE_BREAK
    .brk =
    {
#ifdef CONFIG_LS_TIM1_BREAK1
      .en1 = 1,
      .pol1 = CONFIG_LS_TIM1_BRK1POL,
#endif
#ifdef CONFIG_LS_TIM1_BREAK2
      .en2 = 1,
      .pol2 = CONFIG_LS_TIM1_BRK2POL,
      .flt2 = CONFIG_LS_TIM1_BRK2FLT,
#endif
    },
#endif
#ifdef CONFIG_LS_TIM1_CH1OUT
    .out1 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM1_CH1POL,
      .idle    = CONFIG_LS_TIM1_CH1IDLE,
      .pincfg  = PWM_TIM1_CH1CFG,
    },
#endif
#ifdef CONFIG_LS_TIM1_CH1NOUT
    .out2 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM1_CH1NPOL,
      .idle    = CONFIG_LS_TIM1_CH1NIDLE,
      .pincfg  = PWM_TIM1_CH1NCFG,
    }
#endif
  },
#endif
#ifdef CONFIG_LS_TIM1_CHANNEL2
  {
    .channel = 2,
    .mode    = CONFIG_LS_TIM1_CH2MODE,
#ifdef CONFIG_LS_TIM1_CH2OUT
    .out1 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM1_CH2POL,
      .idle    = CONFIG_LS_TIM1_CH2IDLE,
      .pincfg  = PWM_TIM1_CH2CFG,
    },
#endif
#ifdef CONFIG_LS_TIM1_CH2NOUT
    .out2 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM1_CH2NPOL,
      .idle    = CONFIG_LS_TIM1_CH2NIDLE,
      .pincfg  = PWM_TIM1_CH2NCFG,
    }
#endif
  },
#endif
#ifdef CONFIG_LS_TIM1_CHANNEL3
  {
    .channel = 3,
    .mode    = CONFIG_LS_TIM1_CH3MODE,
#ifdef CONFIG_LS_TIM1_CH3OUT
    .out1 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM1_CH3POL,
      .idle    = CONFIG_LS_TIM1_CH3IDLE,
      .pincfg  = PWM_TIM1_CH3CFG,
    },
#endif
#ifdef CONFIG_LS_TIM1_CH3NOUT
    .out2 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM1_CH3NPOL,
      .idle    = CONFIG_LS_TIM1_CH3NIDLE,
      .pincfg  = PWM_TIM1_CH3NCFG,
    }
#endif
  },
#endif
#ifdef CONFIG_LS_TIM1_CHANNEL4
  {
    .channel = 4,
    .mode    = CONFIG_LS_TIM1_CH4MODE,
#ifdef CONFIG_LS_TIM1_CH4OUT
    .out1 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM1_CH4POL,
      .idle    = CONFIG_LS_TIM1_CH4IDLE,
      .pincfg  = PWM_TIM1_CH4CFG,
    }
#endif
  },
#endif
};

static struct ls_pwmtimer_s g_pwm1dev =
{
  .ops         = &g_pwmops,
#ifdef CONFIG_LS_PWM_LL_OPS
  .llops       = &g_llpwmops,
#endif
  .timid       = 1,
  .chan_num    = PWM_TIM1_NCHANNELS,
  .channels    = g_pwm1channels,
  .timtype     = TIMTYPE_TIM1,
  .mode        = CONFIG_LS_TIM1_MODE,
  .lock        = CONFIG_LS_TIM1_LOCK,
  .t_dts       = CONFIG_LS_TIM1_TDTS,
#ifdef HAVE_PWM_COMPLEMENTARY
  .deadtime    = CONFIG_LS_TIM1_DEADTIME,
#endif
#if defined(HAVE_TRGO) && defined(LS_TIM1_TRGO)
  .trgo        = LS_TIM1_TRGO,
#endif
  .base        = LS_TIM1_BASE,
  .pclk        = TIMCLK_TIM1,
};
#endif /* CONFIG_LS_TIM1_PWM */

#ifdef CONFIG_LS_TIM2_PWM

static struct ls_pwmchan_s g_pwm2channels[] =
{
  /* TIM2 has 4 channels */

#ifdef CONFIG_LS_TIM2_CHANNEL1
  {
    .channel = 1,
    .mode    = CONFIG_LS_TIM2_CH1MODE,
#ifdef CONFIG_LS_TIM2_CH1OUT
    .out1 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM2_CH1POL,
      .idle    = CONFIG_LS_TIM2_CH1IDLE,
      .pincfg  = PWM_TIM2_CH1CFG,
    }
#endif
    /* No complementary outputs */
  },
#endif
#ifdef CONFIG_LS_TIM2_CHANNEL2
  {
    .channel = 2,
    .mode    = CONFIG_LS_TIM2_CH2MODE,
#ifdef CONFIG_LS_TIM2_CH2OUT
    .out1 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM2_CH2POL,
      .idle    = CONFIG_LS_TIM2_CH2IDLE,
      .pincfg  = PWM_TIM2_CH2CFG,
    }
#endif
    /* No complementary outputs */
  },
#endif
#ifdef CONFIG_LS_TIM2_CHANNEL3
  {
    .channel = 3,
    .mode    = CONFIG_LS_TIM2_CH3MODE,
#ifdef CONFIG_LS_TIM2_CH3OUT
    .out1 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM2_CH3POL,
      .idle    = CONFIG_LS_TIM2_CH3IDLE,
      .pincfg  = PWM_TIM2_CH3CFG,
    }
#endif
    /* No complementary outputs */
  },
#endif
#ifdef CONFIG_LS_TIM2_CHANNEL4
  {
    .channel = 4,
    .mode    = CONFIG_LS_TIM2_CH4MODE,
#ifdef CONFIG_LS_TIM2_CH4OUT
    .out1 =
    {
      .in_use  = 1,
      .pol     = CONFIG_LS_TIM2_CH4POL,
      .idle    = CONFIG_LS_TIM2_CH4IDLE,
      .pincfg  = PWM_TIM2_CH4CFG,
    }
#endif
    /* No complementary outputs */
  }
#endif
};

static struct ls_pwmtimer_s g_pwm2dev =
{
  .ops         = &g_pwmops,
#ifdef CONFIG_LS_PWM_LL_OPS
  .llops       = &g_llpwmops,
#endif
  .timid       = 2,
  .chan_num    = PWM_TIM2_NCHANNELS,
  .channels    = g_pwm2channels,
  .timtype     = TIMTYPE_TIM2,
  .mode        = CONFIG_LS_TIM2_MODE,
  .lock        = 0,             /* No lock */
  .t_dts       = 0,             /* No t_dts */
#ifdef HAVE_PWM_COMPLEMENTARY
  .deadtime    = 0,             /* No deadtime */
#endif
#if defined(HAVE_TRGO) && defined(LS_TIM2_TRGO)
  .trgo        = LS_TIM2_TRGO,
#endif
  .base        = LS_TIM2_BASE,
  .pclk        = TIMCLK_TIM2,
};
#endif /* CONFIG_LS_TIM2_PWM */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: pwm_reg_is_32bit
 ****************************************************************************/

static bool pwm_reg_is_32bit(uint8_t timtype, uint32_t offset)
{
  return true;
}

/****************************************************************************
 * Name: pwm_getreg
 *
 * Description:
 *   Read the value of an PWM timer register
 *
 * Input Parameters:
 *   priv   - A reference to the PWM block status
 *   offset - The offset to the register to read
 *
 * Returned Value:
 *   The current contents of the specified register
 *
 ****************************************************************************/

static uint32_t pwm_getreg(struct ls_pwmtimer_s *priv, int offset)
{
  return getreg32(priv->base + offset);
}

/****************************************************************************
 * Name: pwm_putreg
 *
 * Description:
 *   Read the value of an PWM timer register
 *
 * Input Parameters:
 *   priv   - A reference to the PWM block status
 *   offset - The offset to the register to read
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static void pwm_putreg(struct ls_pwmtimer_s *priv, int offset,
                       uint32_t value)
{
  putreg32(value, priv->base + offset);
}

/****************************************************************************
 * Name: pwm_modifyreg
 *
 * Description:
 *   Modify PWM register (32-bit or 16-bit)
 *
 * Input Parameters:
 *   priv    - A reference to the PWM block status
 *   offset  - The offset to the register to read
 *   clrbits - The bits to clear
 *   setbits - The bits to set
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static void pwm_modifyreg(struct ls_pwmtimer_s *priv, uint32_t offset,
                          uint32_t clearbits, uint32_t setbits)
{
  modifyreg32(priv->base + offset, clearbits, setbits);
}

/****************************************************************************
 * Name: pwm_dumpregs
 *
 * Description:
 *   Dump all timer registers.
 *
 * Input Parameters:
 *   dev - A reference to the lower half PWM driver state structure
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#ifdef CONFIG_DEBUG_PWM_INFO
static void pwm_dumpregs(struct pwm_lowerhalf_s *dev, const char *msg)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;

  pwminfo("%s:\n", msg);
  pwminfo("  CR1: %04x CR2:  %04x SMCR:  %04x DIER:  %04x\n",
          pwm_getreg(priv, LS_GTIM_CR1_OFFSET),
          pwm_getreg(priv, LS_GTIM_CR2_OFFSET),
          pwm_getreg(priv, LS_GTIM_SMCR_OFFSET),
          pwm_getreg(priv, LS_GTIM_DIER_OFFSET));

  pwminfo("   SR: %04x EGR:  %04x CCMR1: %04x CCMR2: %04x\n",
          pwm_getreg(priv, LS_GTIM_SR_OFFSET),
          pwm_getreg(priv, LS_GTIM_EGR_OFFSET),
          pwm_getreg(priv, LS_GTIM_CCMR1_OFFSET),
          pwm_getreg(priv, LS_GTIM_CCMR2_OFFSET));

  pwminfo(" CCER: %04x CNT:  %04x PSC:   %04x ARR:   %04x\n",
          pwm_getreg(priv, LS_GTIM_CCER_OFFSET),
          pwm_getreg(priv, LS_GTIM_CNT_OFFSET),
          pwm_getreg(priv, LS_GTIM_PSC_OFFSET),
          pwm_getreg(priv, LS_GTIM_ARR_OFFSET));

  if (priv->timid == 1)
    {
      pwminfo("  RCR: %04x BDTR: %04x\n",
          pwm_getreg(priv, LS_ATIM_RCR_OFFSET),
          pwm_getreg(priv, LS_ATIM_BDTR_OFFSET));
    }

  pwminfo(" CCR1: %04x CCR2: %04x CCR3:  %04x CCR4:  %04x\n",
          pwm_getreg(priv, LS_GTIM_CCR1_OFFSET),
          pwm_getreg(priv, LS_GTIM_CCR2_OFFSET),
          pwm_getreg(priv, LS_GTIM_CCR3_OFFSET),
          pwm_getreg(priv, LS_GTIM_CCR4_OFFSET));

  pwminfo("  DCR: %04x DMAR: %04x\n",
          pwm_getreg(priv, LS_GTIM_DCR_OFFSET),
          pwm_getreg(priv, LS_GTIM_DMAR_OFFSET));
}
#endif

/****************************************************************************
 * Name: pwm_ccr_update
 ****************************************************************************/

static int pwm_ccr_update(struct pwm_lowerhalf_s *dev, uint8_t index,
                          uint32_t ccr)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t offset = 0;

  /* REVISIT: start index from 0? */

  switch (index)
    {
      case LS_PWM_CHAN1:
        {
          offset = LS_GTIM_CCR1_OFFSET;
          break;
        }

      case LS_PWM_CHAN2:
        {
          offset = LS_GTIM_CCR2_OFFSET;
          break;
        }

      case LS_PWM_CHAN3:
        {
          offset = LS_GTIM_CCR3_OFFSET;
          break;
        }

      case LS_PWM_CHAN4:
        {
          offset = LS_GTIM_CCR4_OFFSET;
          break;
        }

      default:
        {
          pwmerr("ERROR: No such CCR: %u\n", index);
          return -EINVAL;
        }
    }

  /* Update CCR register */

  pwm_putreg(priv, offset, ccr);

  return OK;
}

/****************************************************************************
 * Name: pwm_ccr_get
 ****************************************************************************/

#ifdef CONFIG_LS_PWM_LL_OPS
static uint32_t pwm_ccr_get(struct pwm_lowerhalf_s *dev, uint8_t index)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t offset = 0;

  switch (index)
    {
      case LS_PWM_CHAN1:
        {
          offset = LS_GTIM_CCR1_OFFSET;
          break;
        }

      case LS_PWM_CHAN2:
        {
          offset = LS_GTIM_CCR2_OFFSET;
          break;
        }

      case LS_PWM_CHAN3:
        {
          offset = LS_GTIM_CCR3_OFFSET;
          break;
        }

      case LS_PWM_CHAN4:
        {
          offset = LS_GTIM_CCR4_OFFSET;
          break;
        }

      default:
        {
          pwmerr("ERROR: No such CCR: %u\n", index);
          return -EINVAL;
        }
    }

  /* Return CCR register */

  return pwm_getreg(priv, offset);
}
#endif /* CONFIG_LS_PWM_LL_OPS */

/****************************************************************************
 * Name: pwm_arr_update
 ****************************************************************************/

static int pwm_arr_update(struct pwm_lowerhalf_s *dev, uint32_t arr)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;

  /* Update ARR register */

  pwm_putreg(priv, LS_GTIM_ARR_OFFSET, arr);

  return OK;
}

/****************************************************************************
 * Name: pwm_arr_get
 ****************************************************************************/

static uint32_t pwm_arr_get(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;

  return pwm_getreg(priv, LS_GTIM_ARR_OFFSET);
}

#ifdef HAVE_ADVTIM
/****************************************************************************
 * Name: pwm_rcr_update
 ****************************************************************************/

static int pwm_rcr_update(struct pwm_lowerhalf_s *dev, uint16_t rcr)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;

  /* Update RCR register */

  pwm_putreg(priv, LS_ATIM_RCR_OFFSET, rcr);

  return OK;
}
#endif

#ifdef CONFIG_LS_PWM_LL_OPS
/****************************************************************************
 * Name: pwm_rcr_get
 ****************************************************************************/

static uint16_t pwm_rcr_get(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;

  return pwm_getreg(priv, LS_ATIM_RCR_OFFSET);
}
#endif

/****************************************************************************
 * Name: pwm_duty_update
 *
 * Description:
 *   Try to change only channel duty
 *
 * Input Parameters:
 *   dev     - A reference to the lower half PWM driver state structure
 *   channel - Channel to by updated
 *   duty    - New duty
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure
 *
 ****************************************************************************/

static int pwm_duty_update(struct pwm_lowerhalf_s *dev, uint8_t channel,
                           ub16_t duty)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t reload = 0;
  uint32_t ccr    = 0;

  /* We don't want compilation warnings if no DEBUGASSERT */

  UNUSED(priv);

  DEBUGASSERT(priv != NULL);

  pwminfo("TIM%u channel: %u duty: %08" PRIx32 "\n",
          priv->timid, channel, duty);

  /* Get the reload values */

  reload = pwm_arr_get(dev);

  /* Duty cycle:
   *
   * duty cycle = ccr / reload (fractional value)
   */

  ccr = b16toi(duty * reload + b16HALF);

  pwminfo("ccr: %" PRIu32 "\n", ccr);

  /* Write corresponding CCR register */

  pwm_ccr_update(dev, channel, ccr);

  return OK;
}

/****************************************************************************
 * Name: pwm_timer_enable
 ****************************************************************************/

static int pwm_timer_enable(struct pwm_lowerhalf_s *dev, bool state)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;

  if (state == true)
    {
      /* Enable timer counter */

      pwm_modifyreg(priv, LS_GTIM_CR1_OFFSET, 0, GTIM_CR1_CEN);
    }
  else
    {
      /* Disable timer counter */

      pwm_modifyreg(priv, LS_GTIM_CR1_OFFSET, GTIM_CR1_CEN, 0);
    }

  return OK;
}

/****************************************************************************
 * Name: pwm_frequency_update
 *
 * Description:
 *   Update a PWM timer frequency
 *
 ****************************************************************************/

static int pwm_frequency_update(struct pwm_lowerhalf_s *dev,
                                uint32_t frequency)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t reload    = 0;
  uint32_t timclk    = 0;
  uint32_t prescaler = 0;

  /* Calculate optimal values for the timer prescaler and for the timer
   * reload register. If 'frequency' is the desired frequency, then
   *
   *   reload = timclk / frequency
   *   timclk = pclk / presc
   *
   * Or,
   *
   *   reload = pclk / presc / frequency
   *
   * There are many solutions to this, but the best solution will be the one
   * that has the largest reload value and the smallest prescaler value.
   * That is the solution that should give us the most accuracy in the timer
   * control.  Subject to:
   *
   *   0 <= presc  <= 65536
   *   1 <= reload <= 65535
   *
   * So presc = pclk / 65535 / frequency would be optimal.
   *
   * Example:
   *
   *  pclk      = 42 MHz
   *  frequency = 100 Hz
   *
   *  prescaler = 42,000,000 / 65,535 / 100
   *            = 6.4 (or 7 -- taking the ceiling always)
   *  timclk    = 42,000,000 / 7
   *            = 6,000,000
   *  reload    = 6,000,000 / 100
   *            = 60,000
   */

  prescaler = (priv->pclk / frequency + 65534) / 65535;
  if (prescaler < 1)
    {
      prescaler = 1;
    }
  else if (prescaler > 65536)
    {
      prescaler = 65536;
    }

  timclk = priv->pclk / prescaler;

  reload = timclk / frequency;
  if (reload < 2)
    {
      reload = 1;
    }
  else if (reload > 65535)
    {
      reload = 65535;
    }
  else
    {
      reload--;
    }

  pwminfo("TIM%u PCLK: %" PRIu32" frequency: %" PRIu32
          " TIMCLK: %" PRIu32 " "
          "prescaler: %" PRIu32 " reload: %" PRIu32 "\n",
          priv->timid, priv->pclk, frequency, timclk, prescaler, reload);

  /* Set the reload and prescaler values */

  pwm_arr_update(dev, reload);
  pwm_putreg(priv, LS_GTIM_PSC_OFFSET, (uint16_t)(prescaler - 1));

  return OK;
}

/****************************************************************************
 * Name: pwm_timer_configure
 *
 * Description:
 *   Initial configuration for PWM timer
 *
 ****************************************************************************/

static int pwm_timer_configure(struct ls_pwmtimer_s *priv)
{
  uint16_t cr1 = 0;
  int      ret = OK;

  /* Set up the timer CR1 register:
   *
   * 1     CKD[1:0] ARPE CMS[1:0] DIR OPM URS UDIS CEN
   * 2     CKD[1:0] ARPE CMS      DIR OPM URS UDIS CEN
   * 6              ARPE              OPM URS UDIS CEN
   */

  cr1 = pwm_getreg(priv, LS_GTIM_CR1_OFFSET);

  /* Set the counter mode for the advanced timers (1,8) and most general
   * purpose timers (all 2-5, but not 9-17), i.e., all but TIMTYPE_COUNTUP16
   * and TIMTYPE_BASIC
   */

  if (priv->timtype != TIMTYPE_BASIC && priv->timtype != TIMTYPE_COUNTUP16)
    {
      /* Select the Counter Mode:
       *
       * GTIM_CR1_EDGE: The counter counts up or down depending on the
       *   direction bit (DIR).
       * GTIM_CR1_CENTER1, GTIM_CR1_CENTER2, GTIM_CR1_CENTER3: The counter
       *   counts up then down.
       * GTIM_CR1_DIR: 0: count up, 1: count down
       */

      cr1 &= ~(GTIM_CR1_DIR | GTIM_CR1_CMS_MASK);

      switch (priv->mode)
        {
          case LS_TIMMODE_COUNTUP:
            {
              cr1 |= GTIM_CR1_EDGE;
              break;
            }

          case LS_TIMMODE_COUNTDOWN:
            {
              cr1 |= GTIM_CR1_EDGE | GTIM_CR1_DIR;
              break;
            }

          case LS_TIMMODE_CENTER1:
            {
              cr1 |= GTIM_CR1_CENTER1;
              break;
            }

          case LS_TIMMODE_CENTER2:
            {
              cr1 |= GTIM_CR1_CENTER2;
              break;
            }

          case LS_TIMMODE_CENTER3:
            {
              cr1 |= GTIM_CR1_CENTER3;
              break;
            }

          default:
            {
              pwmerr("ERROR: No such timer mode: %u\n",
                     (unsigned int)priv->mode);
              ret = -EINVAL;
              goto errout;
            }
        }
    }

  /* Enable ARR Preload
   * TODO: this should be configurable
   */

  cr1 |= GTIM_CR1_ARPE;

  /* Write CR1 */

  pwm_putreg(priv, LS_GTIM_CR1_OFFSET, cr1);

errout:
  return ret;
}

/****************************************************************************
 * Name: pwm_mode_configure
 *
 * Description:
 *   Configure a PWM mode for given channel
 *
 ****************************************************************************/

static int pwm_mode_configure(struct pwm_lowerhalf_s *dev,
                              uint8_t channel, uint32_t mode)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t chanmode = 0;
  uint32_t ocmode   = 0;
  uint32_t ccmr     = 0;
  uint32_t offset   = 0;
  int      ret      = OK;

  /* Get channel mode
   * TODO: configurable preload for CCxR
   */

  switch (mode)
    {
      case LS_CHANMODE_FRZN:
        {
          chanmode = GTIM_CCMR_MODE_FRZN;
          break;
        }

      case LS_CHANMODE_CHACT:
        {
          chanmode = GTIM_CCMR_MODE_CHACT;
          break;
        }

      case LS_CHANMODE_CHINACT:
        {
          chanmode = GTIM_CCMR_MODE_CHINACT;
          break;
        }

      case LS_CHANMODE_OCREFTOG:
        {
          chanmode = GTIM_CCMR_MODE_OCREFTOG;
          break;
        }

      case LS_CHANMODE_OCREFLO:
        {
          chanmode = GTIM_CCMR_MODE_OCREFLO;
          break;
        }

      case LS_CHANMODE_OCREFHI:
        {
          chanmode = GTIM_CCMR_MODE_OCREFHI;
          break;
        }

      case LS_CHANMODE_PWM1:
        {
          chanmode = GTIM_CCMR_MODE_PWM1;
          break;
        }

      case LS_CHANMODE_PWM2:
        {
          chanmode = GTIM_CCMR_MODE_PWM2;
          break;
        }

      default:
        {
          pwmerr("ERROR: No such mode: %u\n", (unsigned int)mode);
          ret = -EINVAL;
          goto errout;
        }
    }

  /* Get CCMR offset */

  switch (channel)
    {
      case LS_PWM_CHAN1:
      case LS_PWM_CHAN2:
        {
          offset = LS_GTIM_CCMR1_OFFSET;
          break;
        }

      case LS_PWM_CHAN3:
      case LS_PWM_CHAN4:
        {
          offset = LS_GTIM_CCMR2_OFFSET;
          break;
        }

      default:
        {
          pwmerr("ERROR: No such channel: %u\n", channel);
          ret = -EINVAL;
          goto errout;
        }
    }

  /* Get current registers */

  ccmr = pwm_getreg(priv, offset);

  /* PWM mode configuration.
   * NOTE: The CCMRx registers are identical if the channels are outputs.
   */

  switch (channel)
    {
      /* Configure channel 1/3/5 */

      case LS_PWM_CHAN1:
      case LS_PWM_CHAN3:
        {
          /* Reset current channel 1/3/5 mode configuration */

          ccmr &= ~(ATIM_CCMR1_CC1S_MASK | ATIM_CCMR1_OC1M_MASK |
                     ATIM_CCMR1_OC1PE);

          /* Configure CC1/3/5 as output */

          ocmode |= (ATIM_CCMR_CCS_CCOUT << ATIM_CCMR1_CC1S_SHIFT);

          /* Configure Compare 1/3/5 mode */

          ocmode |= (chanmode << ATIM_CCMR1_OC1M_SHIFT);

          /* Enable CCR1/3/5 preload */

          ocmode |= ATIM_CCMR1_OC1PE;

          break;
        }

      /* Configure channel 2/4/6 */

      case LS_PWM_CHAN2:
      case LS_PWM_CHAN4:
        {
          /* Reset current channel 2/4/6 mode configuration */

          ccmr &= ~(ATIM_CCMR1_CC2S_MASK | ATIM_CCMR1_OC2M_MASK |
                     ATIM_CCMR1_OC2PE);

          /* Configure CC2/4/6 as output */

          ocmode |= (ATIM_CCMR_CCS_CCOUT << ATIM_CCMR1_CC2S_SHIFT);

          /* Configure Compare 2/4/6 mode */

          ocmode |= (chanmode << ATIM_CCMR1_OC2M_SHIFT);

          /* Enable CCR2/4/6 preload */

          ocmode |= ATIM_CCMR1_OC2PE;

          break;
        }
    }

  /* Set the selected output compare mode */

  ccmr |= ocmode;

  /* Write CCMRx registers */

  pwm_putreg(priv, offset, ccmr);

errout:
  return ret;
}

/****************************************************************************
 * Name: pwm_output_configure
 *
 * Description:
 *   Configure PWM output for given channel
 *
 ****************************************************************************/

static int pwm_output_configure(struct ls_pwmtimer_s *priv,
                                struct ls_pwmchan_s *chan)
{
  uint32_t cr2  = 0;
  uint32_t ccer = 0;
  uint8_t  channel = 0;

  /* Get channel */

  channel = chan->channel;

  /* Get current registers state */

  cr2  = pwm_getreg(priv, LS_GTIM_CR2_OFFSET);
  ccer = pwm_getreg(priv, LS_GTIM_CCER_OFFSET);

  /* | OISx/OISxN  | IDLE | for ADVANCED and COUNTUP16 | CR2 register
   * | CCxP/CCxNP  | POL  | all PWM timers             | CCER register
   */

  /* Configure output polarity (all PWM timers) */

  if (chan->out1.pol == LS_POL_NEG)
    {
      ccer |= (GTIM_CCER_CC1P << ((channel - 1) * 4));
    }
  else
    {
      ccer &= ~(GTIM_CCER_CC1P << ((channel - 1) * 4));
    }

#ifdef HAVE_ADVTIM
  if (priv->timtype == TIMTYPE_ADVANCED ||
      priv->timtype == TIMTYPE_COUNTUP16_N)
    {
      /* Configure output IDLE State */

      if (chan->out1.idle == LS_IDLE_ACTIVE)
        {
          cr2 |= (ATIM_CR2_OIS1 << ((channel - 1) * 2));
        }
      else
        {
          cr2 &= ~(ATIM_CR2_OIS1 << ((channel - 1) * 2));
        }

#ifdef HAVE_PWM_COMPLEMENTARY
      /* Configure complementary output IDLE state */

      if (chan->out2.idle == LS_IDLE_ACTIVE)
        {
          cr2 |= (ATIM_CR2_OIS1N << ((channel - 1) * 2));
        }
      else
        {
          cr2 &= ~(ATIM_CR2_OIS1N << ((channel - 1) * 2));
        }

      /* Configure complementary output polarity */

      if (chan->out2.pol == LS_POL_NEG)
        {
          ccer |= (ATIM_CCER_CC1NP << ((channel - 1) * 4));
        }
      else
        {
          ccer &= ~(ATIM_CCER_CC1NP << ((channel - 1) * 4));
        }
#endif /* HAVE_PWM_COMPLEMENTARY */
    }
#ifdef HAVE_GTIM_CCXNP
  else
#endif /* HAVE_GTIM_CCXNP */
#endif /* HAVE_ADVTIM */
#ifdef HAVE_GTIM_CCXNP
    {
      /* CCxNP must be cleared if not ADVANCED timer.
       *
       * REVISIT: not all families have CCxNP bits for GTIM,
       *          which causes an ugly condition above
       */

      ccer &= ~(GTIM_CCER_CC1NP << ((channel - 1) * 4));
    }
#endif /* HAVE_GTIM_CCXNP */

  /* Write registers */

  pwm_modifyreg(priv, LS_GTIM_CR2_OFFSET, 0, cr2);
  pwm_modifyreg(priv, LS_GTIM_CCER_OFFSET, 0, ccer);

  return OK;
}

/****************************************************************************
 * Name: pwm_outputs_enable
 *
 * Description:
 *   Enable/disable given timer PWM outputs.
 *
 *   NOTE: This is bulk operation - we can enable/disable many outputs
 *   at one time
 *
 * Input Parameters:
 *   dev     - A reference to the lower half PWM driver state structure
 *   outputs - outputs to set (look at enum ls_chan_e in ls_tim_pwm.h)
 *   state   - Enable/disable operation
 *
 ****************************************************************************/

static int pwm_outputs_enable(struct pwm_lowerhalf_s *dev,
                              uint16_t outputs, bool state)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t ccer   = 0;
  uint32_t regval = 0;

  /* Get current register state */

  ccer = pwm_getreg(priv, LS_GTIM_CCER_OFFSET);

  /* Get outputs configuration */

  regval |= ((outputs & LS_PWM_OUT1)  ? GTIM_CCER_CC1E  : 0);
  regval |= ((outputs & LS_PWM_OUT1N) ? ATIM_CCER_CC1NE : 0);
  regval |= ((outputs & LS_PWM_OUT2)  ? GTIM_CCER_CC2E  : 0);
  regval |= ((outputs & LS_PWM_OUT2N) ? ATIM_CCER_CC2NE : 0);
  regval |= ((outputs & LS_PWM_OUT3)  ? GTIM_CCER_CC3E  : 0);
  regval |= ((outputs & LS_PWM_OUT3N) ? ATIM_CCER_CC3NE : 0);
  regval |= ((outputs & LS_PWM_OUT4)  ? GTIM_CCER_CC4E  : 0);

  /* NOTE: CC4N doesn't exist, but some docs show configuration bits for it */

  if (state == true)
    {
      /* Enable outputs - set bits */

      ccer |= regval;
    }
  else
    {
      /* Disable outputs - reset bits */

      ccer &= ~regval;
    }

  /* Write register */

  pwm_putreg(priv, LS_GTIM_CCER_OFFSET, ccer);

  return OK;
}

#if defined(HAVE_PWM_COMPLEMENTARY) && defined(CONFIG_LS_PWM_LL_OPS)

/****************************************************************************
 * Name: pwm_deadtime_update
 ****************************************************************************/

static int pwm_deadtime_update(struct pwm_lowerhalf_s *dev, uint8_t dt)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t bdtr = 0;
  int      ret  = OK;

  /* Check if locked */

  if (priv->lock > 0)
    {
      ret = -EACCES;
      goto errout;
    }

  /* Get current register state */

  bdtr = pwm_getreg(priv, LS_ATIM_BDTR_OFFSET);

  /* TODO: check if BDTR not locked */

  /* Update deadtime */

  bdtr &= ~(ATIM_BDTR_DTG_MASK);
  bdtr |= (dt << ATIM_BDTR_DTG_SHIFT);

  /* Write BDTR register */

  pwm_putreg(priv, LS_ATIM_BDTR_OFFSET, bdtr);

errout:
  return ret;
}
#endif

#ifdef HAVE_TRGO
/****************************************************************************
 * Name: pwm_trgo_configure
 *
 * Description:
 *   Configure an output synchronisation event for PWM timer (TRGO/TRGO2)
 *
 ****************************************************************************/

static int pwm_trgo_configure(struct pwm_lowerhalf_s *dev,
                              uint8_t trgo)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t cr2 = 0;

  /* Configure TRGO (4 LSB in trgo) */

  cr2 |= (((trgo >> 0) & 0x0f) << ATIM_CR2_MMS_SHIFT) & ATIM_CR2_MMS_MASK;

  /* Write register */

  pwm_modifyreg(priv, LS_GTIM_CR2_OFFSET, 0, cr2);

  return OK;
}
#endif

/****************************************************************************
 * Name: pwm_soft_update
 *
 * Description:
 *   Generate an software update event
 *
 ****************************************************************************/

static int pwm_soft_update(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;

  pwm_putreg(priv, LS_GTIM_EGR_OFFSET, GTIM_EGR_UG);

  return OK;
}

/****************************************************************************
 * Name: pwm_soft_break
 *
 * Description:
 *   Generate an software break event
 *
 *   Outputs are enabled if state is false.
 *   Outputs are disabled if state is true.
 *
 *   NOTE: only timers with complementary outputs have BDTR register and
 *         support software break.
 *
 ****************************************************************************/

static int pwm_soft_break(struct pwm_lowerhalf_s *dev, bool state)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;

  if (state == true)
    {
      /* Reset MOE bit */

      pwm_modifyreg(priv, LS_ATIM_BDTR_OFFSET, ATIM_BDTR_MOE, 0);
    }
  else
    {
      /* Set MOE bit */

      pwm_modifyreg(priv, LS_ATIM_BDTR_OFFSET, 0, ATIM_BDTR_MOE);
    }

  return OK;
}

/****************************************************************************
 * Name: pwm_outputs_from_channels
 *
 * Description:
 *   Get enabled outputs configuration from the PWM timer state
 *
 ****************************************************************************/

static uint16_t pwm_outputs_from_channels(struct ls_pwmtimer_s *priv)
{
  uint16_t outputs = 0;
  uint8_t  channel = 0;
  uint8_t  i       = 0;

  for (i = 0; i < priv->chan_num; i += 1)
    {
      /* Get channel */

      channel = priv->channels[i].channel;

      /* Set outputs if channel configured */

      if (channel != 0)
        {
          /* Enable output if configured */

          if (priv->channels[i].out1.in_use == 1)
            {
              outputs |= (LS_PWM_OUT1 << ((channel - 1) * 2));
            }

#ifdef HAVE_PWM_COMPLEMENTARY
          /* Enable complementary output if configured */

          if (priv->channels[i].out2.in_use == 1)
            {
              outputs |= (LS_PWM_OUT1N << ((channel - 1) * 2));
            }
#endif
        }
    }

  return outputs;
}

#ifdef HAVE_ADVTIM

/****************************************************************************
 * Name: pwm_break_dt_configure
 *
 * Description:
 *   Configure break and deadtime
 *
 * NOTE: we have to configure all BDTR registers at once due to possible
 *       lock configuration
 *
 ****************************************************************************/

static int pwm_break_dt_configure(struct ls_pwmtimer_s *priv)
{
  uint32_t bdtr = 0;

  /* Set the clock division to zero for all (but the basic timers, but there
   * should be no basic timers in this context
   */

  pwm_modifyreg(priv, LS_GTIM_CR1_OFFSET, GTIM_CR1_CKD_MASK,
                priv->t_dts << GTIM_CR1_CKD_SHIFT);

#ifdef HAVE_PWM_COMPLEMENTARY
  /* Initialize deadtime */

  bdtr |= (priv->deadtime << ATIM_BDTR_DTG_SHIFT);
#endif

#ifdef HAVE_BREAK
  /* Configure Break 1 */

  if (priv->brk.en1 == 1)
    {
      /* Enable Break 1 */

      bdtr |= ATIM_BDTR_BKE;

      /* Set Break 1 polarity */

      bdtr |= (priv->brk.pol1 == LS_POL_NEG ? ATIM_BDTR_BKP : 0);
    }
#endif /* HAVE_BREAK */

  /* Clear the OSSI and OSSR bits in the BDTR register.
   *
   * REVISIT: this should be configurable
   */

  bdtr &= ~(ATIM_BDTR_OSSI | ATIM_BDTR_OSSR);

  /* Configure lock */

  bdtr |= priv->lock << ATIM_BDTR_LOCK_SHIFT;

  /* Write BDTR register at once */

  pwm_putreg(priv, LS_ATIM_BDTR_OFFSET, bdtr);

  return OK;
}
#endif

/****************************************************************************
 * Name: pwm_configure
 *
 * Description:
 *   Configure PWM timer in standard mode
 *
 ****************************************************************************/

static int pwm_configure(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint16_t outputs = 0;
  uint8_t j        = 0;
  int     ret      = OK;

  /* NOTE: leave timer counter disabled and all outputs disabled! */

  /* Get configured outputs */

  outputs = pwm_outputs_from_channels(priv);

  /* Disable outputs */

  ret = pwm_outputs_enable(dev, outputs, false);
  if (ret < 0)
    {
      goto errout;
    }

  /* Disable the timer until we get it configured */

  pwm_timer_enable(dev, false);

  /* Initial timer configuration */

  ret = pwm_timer_configure(priv);
  if (ret < 0)
    {
      goto errout;
    }

  /* Some special setup for advanced timers */

#ifdef HAVE_ADVTIM
  if (priv->timtype == TIMTYPE_ADVANCED ||
      priv->timtype == TIMTYPE_COUNTUP16_N)
    {
      /* Configure break and deadtime register */

      ret = pwm_break_dt_configure(priv);
      if (ret < 0)
        {
          goto errout;
        }

#ifdef HAVE_TRGO
      /* Configure TRGO/TRGO2 */

      ret = pwm_trgo_configure(dev, priv->trgo);
      if (ret < 0)
        {
          goto errout;
        }
#endif
    }
#endif

  /* Configure timer channels */

  for (j = 0; j < priv->chan_num; j++)
    {
      /* Skip channel if not in use */

      if (priv->channels[j].channel != 0)
        {
          /* Update PWM mode */

          ret = pwm_mode_configure(dev, priv->channels[j].channel,
                                   priv->channels[j].mode);
          if (ret < 0)
            {
              goto errout;
            }

          /* PWM outputs configuration */

          ret = pwm_output_configure(priv, &priv->channels[j]);
          if (ret < 0)
            {
              goto errout;
            }
        }
    }

  /* Disable software break at the end of the outputs configuration (enablei
   * outputs).
   *
   * NOTE: Only timers with complementary outputs have BDTR register and
   *       support software break.
   */

  if (priv->timtype == TIMTYPE_ADVANCED ||
      priv->timtype == TIMTYPE_COUNTUP16_N)
    {
      ret = pwm_soft_break(dev, false);
      if (ret < 0)
        {
          goto errout;
        }
    }

errout:
  return ret;
}

/****************************************************************************
 * Name: pwm_duty_channels_update
 *
 * Description:
 *   Update duty cycle for given channels
 *
 ****************************************************************************/

static int pwm_duty_channels_update(struct pwm_lowerhalf_s *dev,
                                    const struct pwm_info_s *info)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint8_t   channel = 0;
  ub16_t    duty    = 0;
  int       ret     = OK;
  int       i       = 0;
  int       j       = 0;

  for (i = 0; i < CONFIG_PWM_NCHANNELS; i++)
    {
      /* Break the loop if all following channels are not configured */

      if (info->channels[i].channel == -1)
        {
          break;
        }

      duty    = info->channels[i].duty;
      channel = info->channels[i].channel;

      /* A value of zero means to skip this channel */

      if (channel != 0)
        {
          /* Find the channel */

          for (j = 0; j < priv->chan_num; j++)
            {
              if (priv->channels[j].channel == channel)
                {
                  break;
                }
            }

          /* Check range */

          if (j >= priv->chan_num)
            {
              pwmerr("ERROR: No such channel: %u\n", channel);
              ret = -EINVAL;
              goto errout;
            }

          /* Update duty cycle */

          ret = pwm_duty_update(dev, channel, duty);
          if (ret < 0)
            {
              goto errout;
            }
        }
    }

errout:
  return OK;
}

/****************************************************************************
 * Name: pwm_timer
 *
 * Description:
 *   (Re-)initialize the timer resources and start the pulsed output
 *
 * Input Parameters:
 *   dev  - A reference to the lower half PWM driver state structure
 *   info - A reference to the characteristics of the pulsed output
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure
 *
 ****************************************************************************/

static int pwm_timer(struct pwm_lowerhalf_s *dev,
                     const struct pwm_info_s *info)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint16_t outputs = 0;
  int      ret     = OK;

  DEBUGASSERT(priv != NULL && info != NULL);

  pwminfo("TIM%u frequency: %" PRIu32 "\n",
          priv->timid, info->frequency);

  DEBUGASSERT(info->frequency > 0);

  /* TODO: what if we have pwm running and we want disable some channels ? */

  /* Set timer frequency */

  ret = pwm_frequency_update(dev, info->frequency);
  if (ret < 0)
    {
      goto errout;
    }

  /* Channel specific configuration */

  ret = pwm_duty_channels_update(dev, info);
  if (ret < 0)
    {
      goto errout;
    }

  /* Set the advanced timer's repetition counter */

#ifdef HAVE_ADVTIM
  if (priv->timtype == TIMTYPE_ADVANCED ||
      priv->timtype == TIMTYPE_COUNTUP16_N)
    {
      /* If a non-zero repetition count has been selected, then set the
       * repetition counter to the count-1 (pwm_start() has already
       * assured us that the count value is within range).
       */

      /* Set the repetition counter to zero */

      pwm_rcr_update(dev, 0);

      /* Generate an update event to reload the prescaler */

      pwm_soft_update(dev);
    }
  else
#endif
    {
      /* Generate an update event to reload the prescaler (all timers) */

      pwm_soft_update(dev);
    }

  /* Get configured outputs */

  outputs = pwm_outputs_from_channels(priv);

  /* Enable outputs */

  ret = pwm_outputs_enable(dev, outputs, true);
  if (ret < 0)
    {
      goto errout;
    }

  /* Just enable the timer, leaving all interrupts disabled */

  pwm_timer_enable(dev, true);

  pwm_dumpregs(dev, "After starting");

errout:
  return ret;
}

/****************************************************************************
 * Name: pwm_set_apb_clock
 *
 * Description:
 *   Enable or disable APB clock for the timer peripheral
 *
 * Input Parameters:
 *   priv - A reference to the PWM block status
 *   on   - Enable clock if 'on' is 'true' and disable if 'false'
 *
 ****************************************************************************/

static int pwm_set_apb_clock(struct ls_pwmtimer_s *priv, bool on)
{
  /* LS2K0300 TIM clocks are always supplied by the APB clock domain. */

  (void)priv;
  (void)on;

  pwminfo("timer %d clock enable: %d\n", priv->timid, on ? 1 : 0);

  return OK;
}

/****************************************************************************
 * Name: pwm_setup
 *
 * Description:
 *   This method is called when the driver is opened.  The lower half driver
 *   should configure and initialize the device so that it is ready for use.
 *   It should not, however, output pulses until the start method is called.
 *
 * Input Parameters:
 *   dev - A reference to the lower half PWM driver state structure
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure
 *
 * Assumptions:
 *   APB1 or 2 clocking for the GPIOs has already been configured by the RCC
 *   logic at power up.
 *
 ****************************************************************************/

static int pwm_setup(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t pincfg = 0;
  int      ret    = OK;
  int      i      = 0;

  pwminfo("TIM%u\n", priv->timid);

  /* Enable APB1/2 clocking for timer. */

  ret = pwm_set_apb_clock(priv, true);
  if (ret < 0)
    {
      goto errout;
    }

  pwm_dumpregs(dev, "Initially");

  /* Configure the PWM output pins, but do not start the timer yet */

  for (i = 0; i < priv->chan_num; i++)
    {
      if (priv->channels[i].out1.in_use == 1)
        {
          /* Do not configure the pin if pincfg is not specified.
           * This prevents overwriting the PA0 configuration if the
           * channel is used internally.
           */

          pincfg = priv->channels[i].out1.pincfg;
          if (pincfg != 0)
            {
              pwminfo("pincfg: %08" PRIx32 "\n", pincfg);

              ls_configgpio(pincfg);
            }
        }

#ifdef HAVE_PWM_COMPLEMENTARY
      if (priv->channels[i].out2.in_use == 1)
        {
          pincfg = priv->channels[i].out2.pincfg;

          /* Do not configure the pin if pincfg is not specified.
           * This prevents overwriting the PA0 configuration if the
           * channel is used internally.
           */

          if (pincfg != 0)
            {
              pwminfo("pincfg: %08" PRIx32 "\n", pincfg);

              ls_configgpio(pincfg);
            }
        }
#endif
    }

  /* Configure PWM timer with the selected configuration.
   *
   * NOTE: We configure PWM here during setup, but leave timer with disabled
   *       counter, disabled outputs, not configured frequency and duty cycle
   */

    {
      ret = pwm_configure(dev);
    }

  if (ret < 0)
    {
      pwmerr("failed to configure PWM %d\n", priv->timid);
      ret = ERROR;
      goto errout;
    }

errout:
  return ret;
}

/****************************************************************************
 * Name: pwm_shutdown
 *
 * Description:
 *   This method is called when the driver is closed.  The lower half driver
 *   stop pulsed output, free any resources, disable the timer hardware, and
 *   put the system into the lowest possible power usage state
 *
 * Input Parameters:
 *   dev - A reference to the lower half PWM driver state structure
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure
 *
 ****************************************************************************/

static int pwm_shutdown(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  uint32_t pincfg = 0;
  int      i      = 0;
  int      ret    = OK;

  pwminfo("TIM%u\n", priv->timid);

  /* Make sure that the output has been stopped */

  pwm_stop(dev);

  /* Disable APB1/2 clocking for timer. */

  ret = pwm_set_apb_clock(priv, false);
  if (ret < 0)
    {
      goto errout;
    }

  /* Then put the GPIO pins back to the default state */

  for (i = 0; i < priv->chan_num; i++)
    {
      pincfg = priv->channels[i].out1.pincfg;
      if (pincfg != 0)
        {
          pwminfo("pincfg: %08" PRIx32 "\n", pincfg);

          ls_unconfiggpio(pincfg);
        }

#ifdef HAVE_PWM_COMPLEMENTARY
      pincfg = priv->channels[i].out2.pincfg;
      if (pincfg != 0)
        {
          pwminfo("pincfg: %08" PRIx32 "\n", pincfg);

          ls_unconfiggpio(pincfg);
        }
#endif
    }

errout:
  return ret;
}

/****************************************************************************
 * Name: pwm_start
 *
 * Description:
 *   (Re-)initialize the timer resources and start the pulsed output
 *
 * Input Parameters:
 *   dev  - A reference to the lower half PWM driver state structure
 *   info - A reference to the characteristics of the pulsed output
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure
 *
 ****************************************************************************/

static int pwm_start(struct pwm_lowerhalf_s *dev,
                     const struct pwm_info_s *info)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  int ret = OK;

  /* if frequency has not changed we just update duty */

  if (info->frequency == priv->frequency)
    {
      int i;

      for (i = 0; ret == OK && i < CONFIG_PWM_NCHANNELS; i++)
        {
          /* Break the loop if all following channels are not configured */

          if (info->channels[i].channel == -1)
            {
              break;
            }

          /* Set output if channel configured */

          if (info->channels[i].channel != 0)
            {
              ret = pwm_duty_update(dev, info->channels[i].channel,
                                    info->channels[i].duty);
            }
        }
    }
  else
    {
      ret = pwm_timer(dev, info);

      /* Save current frequency */

      if (ret == OK)
        {
          priv->frequency = info->frequency;
        }
    }

  return ret;
}

/****************************************************************************
 * Name: pwm_stop
 *
 * Description:
 *   Stop the pulsed output and reset the timer resources
 *
 * Input Parameters:
 *   dev - A reference to the lower half PWM driver state structure
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure
 *
 * Assumptions:
 *   This function is called to stop the pulsed output at anytime.  This
 *   method is also called from the timer interrupt handler when a repetition
 *   count expires... automatically stopping the timer.
 *
 ****************************************************************************/

static int pwm_stop(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;
  irqstate_t flags    = 0;
  uint16_t   outputs  = 0;
  int        ret = OK;

  pwminfo("TIM%u\n", priv->timid);

  /* Disable interrupts momentary to stop any ongoing timer processing and
   * to prevent any concurrent access to the reset register.
   */

  flags = enter_critical_section();

  /* Stopped so frequency is zero */

  priv->frequency = 0;

  /* Disable further interrupts and stop the timer */

  pwm_putreg(priv, LS_GTIM_DIER_OFFSET, 0);
  pwm_putreg(priv, LS_GTIM_SR_OFFSET, 0);

  /* Disable the timer and timer outputs */

  pwm_timer_enable(dev, false);
  outputs = pwm_outputs_from_channels(priv);
  ret = pwm_outputs_enable(dev, outputs, false);

  /* Clear all channels */

  pwm_putreg(priv, LS_GTIM_CCR1_OFFSET, 0);
  pwm_putreg(priv, LS_GTIM_CCR2_OFFSET, 0);
  pwm_putreg(priv, LS_GTIM_CCR3_OFFSET, 0);
  pwm_putreg(priv, LS_GTIM_CCR4_OFFSET, 0);

  leave_critical_section(flags);

  pwm_dumpregs(dev, "After stop");

  return ret;
}

/****************************************************************************
 * Name: pwm_ioctl
 *
 * Description:
 *   Lower-half logic may support platform-specific ioctl commands
 *
 * Input Parameters:
 *   dev - A reference to the lower half PWM driver state structure
 *   cmd - The ioctl command
 *   arg - The argument accompanying the ioctl command
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure
 *
 ****************************************************************************/

static int pwm_ioctl(struct pwm_lowerhalf_s *dev, int cmd,
                     unsigned long arg)
{
#ifdef CONFIG_DEBUG_PWM_INFO
  struct ls_pwmtimer_s *priv = (struct ls_pwmtimer_s *)dev;

  /* There are no platform-specific ioctl commands */

  pwminfo("TIM%u\n", priv->timid);
#endif
  return -ENOTTY;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_tim_pwminitialize
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

struct pwm_lowerhalf_s *ls_tim_pwminitialize(int timer)
{
  struct ls_pwmtimer_s *lower = NULL;

  pwminfo("TIM%u\n", timer);

  switch (timer)
    {
#ifdef CONFIG_LS_TIM1_PWM
      case 1:
        {
          lower = &g_pwm1dev;

          /* Attach but disable the TIM1 update interrupt */

          break;
        }
#endif

#ifdef CONFIG_LS_TIM2_PWM
      case 2:
        {
          lower = &g_pwm2dev;
          break;
        }
#endif

      default:
        {
          pwmerr("ERROR: No such timer configured %d\n", timer);
          lower = NULL;
          goto errout;
        }
    }

errout:
  return (struct pwm_lowerhalf_s *)lower;
}

#endif /* CONFIG_LS_TIM_PWM */
