/****************************************************************************
 * arch/loongarch/src/ls2k/ls_qencoder.c
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

#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <nuttx/debug.h>
#include <inttypes.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <nuttx/sensors/qencoder.h>

#include <arch/board/board.h>

#include "chip.h"
#include "loongarch_internal.h"
#include "ls.h"
#include "ls_gpio.h"
#include "ls_tim.h"
#include "ls_qencoder.h"

#ifdef CONFIG_SENSORS_QENCODER

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Timers *******************************************************************/

/* Input filter *************************************************************/

#ifdef CONFIG_LS_QENCODER_FILTER
#  if defined(CONFIG_LS_QENCODER_SAMPLE_FDTS)
#    if defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_1)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_NOFILT
#    endif
#  elif defined(CONFIG_LS_QENCODER_SAMPLE_CKINT)
#    if defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_2)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FCKINT2
#    elif defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_4)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FCKINT4
#    elif defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_8)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FCKINT8
#    endif
#  elif defined(CONFIG_LS_QENCODER_SAMPLE_FDTS_2)
#    if defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_6)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd26
#    elif defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_8)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd28
#    endif
#  elif defined(CONFIG_LS_QENCODER_SAMPLE_FDTS_4)
#    if defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_6)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd46
#    elif defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_8)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd48
#    endif
#  elif defined(CONFIG_LS_QENCODER_SAMPLE_FDTS_8)
#    if defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_6)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd86
#    elif defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_8)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd88
#    endif
#  elif defined(CONFIG_LS_QENCODER_SAMPLE_FDTS_16)
#    if defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_5)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd165
#    elif defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_6)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd166
#    elif defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_8)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd168
#    endif
#  elif defined(CONFIG_LS_QENCODER_SAMPLE_FDTS_32)
#    if defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_5)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd325
#    elif defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_6)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd326
#    elif defined(CONFIG_LS_QENCODER_SAMPLE_EVENT_8)
#      define LS_QENCODER_ICF GTIM_CCMR_ICF_FDTSd328
#    endif
#  endif

#  ifndef LS_QENCODER_ICF
#    warning "Invalid encoder filter combination, filter disabled"
#  endif
#endif

#ifndef LS_QENCODER_ICF
#  define LS_QENCODER_ICF GTIM_CCMR_ICF_NOFILT
#endif

/* Debug ********************************************************************/

/* Non-standard debug that may be enabled just for testing the quadrature
 * encoder
 */

#ifndef CONFIG_DEBUG_FEATURES
#  undef CONFIG_DEBUG_SENSORS
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* Constant configuration structure that is retained in FLASH */

struct ls_qeconfig_s
{
  uint8_t   timid;   /* Timer ID {1,2} */
  uint32_t  ti1cfg;  /* TI1 input pin configuration */
  uint32_t  ti2cfg;  /* TI2 input pin configuration */
  uintptr_t base;    /* Register base address */
  uint32_t  psc;     /* Timer input clock prescaler */
};

/* Overall, RAM-based state structure */

struct ls_lowerhalf_s
{
  /* The first field of this state structure must be a pointer to the lower-
   * half callback structure:
   */

  const struct qe_ops_s *ops;  /* Lower half callback structure */

  /* LS driver-specific fields: */

  const struct ls_qeconfig_s *config; /* static configuration */

  bool             inuse;        /* True: The lower-half driver is in-use */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* Helper functions */

static uint32_t ls_getreg32(struct ls_lowerhalf_s *priv,
                            int offset);
static void ls_putreg32(struct ls_lowerhalf_s *priv, int offset,
                        uint32_t value);

#if defined(CONFIG_DEBUG_SENSORS) && defined(CONFIG_DEBUG_INFO)
static void ls_dumpregs(struct ls_lowerhalf_s *priv,
                        const char *msg);
#else
#  define ls_dumpregs(priv,msg)
#endif

static struct ls_lowerhalf_s *ls_tim2lower(int tim);

/* Lower-half Quadrature Encoder Driver Methods */

static int ls_setup(struct qe_lowerhalf_s *lower);
static int ls_shutdown(struct qe_lowerhalf_s *lower);
static int ls_position(struct qe_lowerhalf_s *lower, int32_t *pos);
static int ls_setposmax(struct qe_lowerhalf_s *lower, uint32_t pos);
static int ls_reset(struct qe_lowerhalf_s *lower);
static int ls_setindex(struct qe_lowerhalf_s *lower, uint32_t pos);
static int ls_ioctl(struct qe_lowerhalf_s *lower, int cmd,
                    unsigned long arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* The lower half callback structure */

static const struct qe_ops_s g_qecallbacks =
{
  .setup     = ls_setup,
  .shutdown  = ls_shutdown,
  .position  = ls_position,
  .setposmax = ls_setposmax,
  .reset     = ls_reset,
  .setindex  = ls_setindex,
  .ioctl     = ls_ioctl,
};

/* Per-timer state structures */

#ifdef CONFIG_LS_TIM1_QE
static const struct ls_qeconfig_s g_tim1config =
{
  .timid    = 1,
  .base     = LS_TIM1_BASE,
  .psc      = CONFIG_LS_TIM1_QEPSC,
  .ti1cfg   = GPIO_TIM1_CH1IN,
  .ti2cfg   = GPIO_TIM1_CH2IN,
};

static struct ls_lowerhalf_s g_tim1lower =
{
  .ops      = &g_qecallbacks,
  .config   = &g_tim1config,
  .inuse    = false,
};

#endif

#ifdef CONFIG_LS_TIM2_QE
static const struct ls_qeconfig_s g_tim2config =
{
  .timid    = 2,
  .base     = LS_TIM2_BASE,
  .psc      = CONFIG_LS_TIM2_QEPSC,
  .ti1cfg   = GPIO_TIM2_CH1IN,
  .ti2cfg   = GPIO_TIM2_CH2IN,
};

static struct ls_lowerhalf_s g_tim2lower =
{
  .ops      = &g_qecallbacks,
  .config   = &g_tim2config,
  .inuse    = false,
};

#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_getreg32
 *
 * Description:
 *   Read the value of a 32-bit timer register.
 *
 * Input Parameters:
 *   priv - A reference to the lower half status
 *   offset - The offset to the register to read
 *
 * Returned Value:
 *   The current contents of the specified register
 *
 ****************************************************************************/

static uint32_t ls_getreg32(struct ls_lowerhalf_s *priv,
                            int offset)
{
  return getreg32(priv->config->base + offset);
}

/****************************************************************************
 * Name: ls_putreg32
 *
 * Description:
 *   Write a value to a 32-bit timer register.
 *
 * Input Parameters:
 *   priv - A reference to the lower half status
 *   offset - The offset to the register to read
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static void ls_putreg32(struct ls_lowerhalf_s *priv, int offset,
                        uint32_t value)
{
  putreg32(value, priv->config->base + offset);
}

/****************************************************************************
 * Name: ls_dumpregs
 *
 * Description:
 *   Dump all timer registers.
 *
 * Input Parameters:
 *   priv - A reference to the QENCODER block status
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#if defined(CONFIG_DEBUG_SENSORS) && defined(CONFIG_DEBUG_INFO)
static void ls_dumpregs(struct ls_lowerhalf_s *priv,
                        const char *msg)
{
  sninfo("%s:\n", msg);
  sninfo("  CR1: %08" PRIx32 " CR2:  %08" PRIx32
         " SMCR:  %08" PRIx32 " DIER:  %08" PRIx32 "\n",
         ls_getreg32(priv, LS_GTIM_CR1_OFFSET),
         ls_getreg32(priv, LS_GTIM_CR2_OFFSET),
         ls_getreg32(priv, LS_GTIM_SMCR_OFFSET),
         ls_getreg32(priv, LS_GTIM_DIER_OFFSET));
  sninfo("   SR: %08" PRIx32 " EGR:  %08" PRIx32
         " CCMR1: %08" PRIx32 " CCMR2: %08" PRIx32 "\n",
         ls_getreg32(priv, LS_GTIM_SR_OFFSET),
         ls_getreg32(priv, LS_GTIM_EGR_OFFSET),
         ls_getreg32(priv, LS_GTIM_CCMR1_OFFSET),
         ls_getreg32(priv, LS_GTIM_CCMR2_OFFSET));
  sninfo(" CCER: %08" PRIx32 " CNT:  %08" PRIx32
         " PSC:   %08" PRIx32 " ARR:   %08" PRIx32 "\n",
         ls_getreg32(priv, LS_GTIM_CCER_OFFSET),
         ls_getreg32(priv, LS_GTIM_CNT_OFFSET),
         ls_getreg32(priv, LS_GTIM_PSC_OFFSET),
         ls_getreg32(priv, LS_GTIM_ARR_OFFSET));
  sninfo(" CCR1: %08" PRIx32 " CCR2: %08" PRIx32
         " CCR3:  %08" PRIx32 " CCR4:  %08" PRIx32 "\n",
         ls_getreg32(priv, LS_GTIM_CCR1_OFFSET),
         ls_getreg32(priv, LS_GTIM_CCR2_OFFSET),
         ls_getreg32(priv, LS_GTIM_CCR3_OFFSET),
         ls_getreg32(priv, LS_GTIM_CCR4_OFFSET));
#ifdef CONFIG_LS_TIM1_QE
  if (priv->config->timid == 1)
    {
      sninfo("  RCR: %08" PRIx32 " BDTR: %08" PRIx32
             " DCR:   %08" PRIx32 " DMAR:  %08" PRIx32 "\n",
             ls_getreg32(priv, LS_ATIM_RCR_OFFSET),
             ls_getreg32(priv, LS_ATIM_BDTR_OFFSET),
             ls_getreg32(priv, LS_ATIM_DCR_OFFSET),
             ls_getreg32(priv, LS_ATIM_DMAR_OFFSET));
    }
  else
#endif
    {
      sninfo("  DCR: %08" PRIx32 " DMAR: %08" PRIx32 "\n",
             ls_getreg32(priv, LS_GTIM_DCR_OFFSET),
             ls_getreg32(priv, LS_GTIM_DMAR_OFFSET));
    }
}
#endif

/****************************************************************************
 * Name: ls_tim2lower
 *
 * Description:
 *   Map a timer number to a device structure
 *
 ****************************************************************************/

static struct ls_lowerhalf_s *ls_tim2lower(int tim)
{
  switch (tim)
    {
#ifdef CONFIG_LS_TIM1_QE
    case 1:
      return &g_tim1lower;
#endif
#ifdef CONFIG_LS_TIM2_QE
    case 2:
      return &g_tim2lower;
#endif
    default:
      return NULL;
    }
}

/****************************************************************************
 * Name: ls_setup
 *
 * Description:
 *   This method is called when the driver is opened.  The lower half driver
 *   should configure and initialize the device so that it is ready for use.
 *   The initial position value should be zero. *
 *
 ****************************************************************************/

static int ls_setup(struct qe_lowerhalf_s *lower)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;
  uint32_t dier;
  uint32_t smcr;
  uint32_t ccmr1;
  uint32_t ccer;
  uint32_t cr1;

  /* Timer base configuration */

  cr1 = ls_getreg32(priv, LS_GTIM_CR1_OFFSET);

  /* Clear the direction bit (0=count up) and select the Counter Mode
   * (0=Edge aligned) (Timers 2-5 and 1-8 only)
   */

  cr1 &= ~(GTIM_CR1_DIR | GTIM_CR1_CMS_MASK);
  ls_putreg32(priv, LS_GTIM_CR1_OFFSET, cr1);

  /* Set the Autoreload value */

  ls_putreg32(priv, LS_GTIM_ARR_OFFSET, 0xffffffff);

  /* Set the timer prescaler value.
   *
   * If we are doing precise shaft positioning, each qe pulse is important.
   * So the LS has direct config control on the pulse count prescaler.
   * This input clock just limits the incoming pulse rate, which should be
   * lower than the peripheral clock due to resynchronization, but it is the
   * responsibility of the system designer to decide the correct prescaler
   * value, because it has a direct influence on the encoder resolution.
   */

  ls_putreg32(priv, LS_GTIM_PSC_OFFSET, priv->config->psc);

#ifdef CONFIG_LS_TIM1_QE
  if (priv->config->timid == 1)
    {
      /* Clear the Repetition Counter value */

      ls_putreg32(priv, LS_ATIM_RCR_OFFSET, 0);
    }
#endif

  /* Generate an update event to reload the Prescaler
   * and the repetition counter (only for TIM1) value immediately
   */

  ls_putreg32(priv, LS_GTIM_EGR_OFFSET, GTIM_EGR_UG);

  /* GPIO pin configuration */

  ls_configgpio(priv->config->ti1cfg);
  ls_configgpio(priv->config->ti2cfg);

  /* Set the encoder Mode 3 */

  smcr  = ls_getreg32(priv, LS_GTIM_SMCR_OFFSET);
  smcr &= ~GTIM_SMCR_SMS_MASK;
  smcr |= GTIM_SMCR_ENCMD3;
  ls_putreg32(priv, LS_GTIM_SMCR_OFFSET, smcr);

  /* TI1 Channel Configuration */

  /* Disable the Channel 1: Reset the CC1E Bit */

  ccer  = ls_getreg32(priv, LS_GTIM_CCER_OFFSET);
  ccer &= ~GTIM_CCER_CC1E;
  ls_putreg32(priv, LS_GTIM_CCER_OFFSET, ccer);

  ccmr1 = ls_getreg32(priv, LS_GTIM_CCMR1_OFFSET);
  ccer  = ls_getreg32(priv, LS_GTIM_CCER_OFFSET);

  /* Select the Input IC1=TI1 and set the filter fSAMPLING=fDTS/4, N=6 */

  ccmr1 &= ~(GTIM_CCMR1_CC1S_MASK | GTIM_CCMR1_IC1F_MASK);
  ccmr1 |= GTIM_CCMR_CCS_CCIN1 << GTIM_CCMR1_CC1S_SHIFT;
  ccmr1 |= LS_QENCODER_ICF << GTIM_CCMR1_IC1F_SHIFT;

  /* Select the Polarity=rising and set the CC1E Bit */

#ifdef HAVE_GTIM_CCXNP
  ccer &= ~(GTIM_CCER_CC1P | GTIM_CCER_CC1NP);
#else
  ccer &= ~(GTIM_CCER_CC1P);
#endif
  ccer |= GTIM_CCER_CC1E;

  /* Write to TIM CCMR1 and CCER registers */

  ls_putreg32(priv, LS_GTIM_CCMR1_OFFSET, ccmr1);
  ls_putreg32(priv, LS_GTIM_CCER_OFFSET, ccer);

  /* Set the Input Capture Prescaler value: Capture performed each time an
   * edge is detected on the capture input.
   */

  ccmr1  = ls_getreg32(priv, LS_GTIM_CCMR1_OFFSET);
  ccmr1 &= ~GTIM_CCMR1_IC1PSC_MASK;
  ccmr1 |= (GTIM_CCMR_ICPSC_NOPSC << GTIM_CCMR1_IC1PSC_SHIFT);
  ls_putreg32(priv, LS_GTIM_CCMR1_OFFSET, ccmr1);

  /* TI2 Channel Configuration */

  /* Disable the Channel 2: Reset the CC2E Bit */

  ccer  = ls_getreg32(priv, LS_GTIM_CCER_OFFSET);
  ccer &= ~GTIM_CCER_CC2E;
  ls_putreg32(priv, LS_GTIM_CCER_OFFSET, ccer);

  ccmr1 = ls_getreg32(priv, LS_GTIM_CCMR1_OFFSET);
  ccer  = ls_getreg32(priv, LS_GTIM_CCER_OFFSET);

  /* Select the Input IC2=TI2 and set the filter fSAMPLING=fDTS/4, N=6 */

  ccmr1 &= ~(GTIM_CCMR1_CC2S_MASK | GTIM_CCMR1_IC2F_MASK);
  ccmr1 |= GTIM_CCMR_CCS_CCIN1 << GTIM_CCMR1_CC2S_SHIFT;
  ccmr1 |= LS_QENCODER_ICF << GTIM_CCMR1_IC2F_SHIFT;

  /* Select the Polarity=rising and set the CC2E Bit */

#ifdef HAVE_GTIM_CCXNP
  ccer &= ~(GTIM_CCER_CC2P | GTIM_CCER_CC2NP);
#else
  ccer &= ~(GTIM_CCER_CC2P);
#endif
  ccer |= GTIM_CCER_CC2E;

  /* Write to TIM CCMR1 and CCER registers */

  ls_putreg32(priv, LS_GTIM_CCMR1_OFFSET, ccmr1);
  ls_putreg32(priv, LS_GTIM_CCER_OFFSET, ccer);

  /* Set the Input Capture Prescaler value: Capture performed each time an
   * edge is detected on the capture input.
   */

  ccmr1  = ls_getreg32(priv, LS_GTIM_CCMR1_OFFSET);
  ccmr1 &= ~GTIM_CCMR1_IC2PSC_MASK;
  ccmr1 |= (GTIM_CCMR_ICPSC_NOPSC << GTIM_CCMR1_IC2PSC_SHIFT);
  ls_putreg32(priv, LS_GTIM_CCMR1_OFFSET, ccmr1);

  /* Disable the update interrupt */

  dier  = ls_getreg32(priv, LS_GTIM_DIER_OFFSET);
  dier &= ~GTIM_DIER_UIE;
  ls_putreg32(priv, LS_GTIM_DIER_OFFSET, dier);

  /* Reset the Update Disable Bit */

  cr1 = ls_getreg32(priv, LS_GTIM_CR1_OFFSET);
  cr1 &= ~GTIM_CR1_UDIS;
  ls_putreg32(priv, LS_GTIM_CR1_OFFSET, cr1);

  /* Reset the URS Bit */

  cr1 &= ~GTIM_CR1_URS;
  ls_putreg32(priv, LS_GTIM_CR1_OFFSET, cr1);

  /* Enable the TIM Counter */

  cr1 = ls_getreg32(priv, LS_GTIM_CR1_OFFSET);
  cr1 |= GTIM_CR1_CEN;
  ls_putreg32(priv, LS_GTIM_CR1_OFFSET, cr1);

  ls_dumpregs(priv, "After setup");

  return OK;
}

/****************************************************************************
 * Name: ls_shutdown
 *
 * Description:
 *   This method is called when the driver is closed.  The lower half driver
 *   should stop data collection, free any resources, disable timer hardware,
 *   and put the system into the lowest possible power usage state
 *
 ****************************************************************************/

static int ls_shutdown(struct qe_lowerhalf_s *lower)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;
  irqstate_t flags;

  flags = enter_critical_section();

  /* Disable interrupts momentary to stop any ongoing timer processing. */

  /* Disable further interrupts and stop the timer */

  ls_putreg32(priv, LS_GTIM_DIER_OFFSET, 0);
  ls_putreg32(priv, LS_GTIM_SR_OFFSET, 0);
  ls_putreg32(priv, LS_GTIM_CR1_OFFSET, 0);
  ls_putreg32(priv, LS_GTIM_SMCR_OFFSET, 0);
  ls_putreg32(priv, LS_GTIM_CCMR1_OFFSET, 0);
  ls_putreg32(priv, LS_GTIM_CCER_OFFSET, 0);
  ls_putreg32(priv, LS_GTIM_CNT_OFFSET, 0);
  leave_critical_section(flags);

  ls_dumpregs(priv, "After stop");
  return OK;
}

/****************************************************************************
 * Name: ls_position
 *
 * Description:
 *   Return the current position measurement.
 *
 ****************************************************************************/

static int ls_position(struct qe_lowerhalf_s *lower, int32_t *pos)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;

  /* Return the counter value */

  *pos = (int32_t)ls_getreg32(priv, LS_GTIM_CNT_OFFSET);
  return OK;
}

/****************************************************************************
 * Name: ls_setposmax
 *
 * Description:
 *   Set the maximum encoder position.
 *
 ****************************************************************************/

static int ls_setposmax(struct qe_lowerhalf_s *lower, uint32_t pos)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;

  ls_putreg32(priv, LS_GTIM_ARR_OFFSET, pos);

  return OK;
}

/****************************************************************************
 * Name: ls_reset
 *
 * Description:
 *   Reset the position measurement to zero.
 *
 ****************************************************************************/

static int ls_reset(struct qe_lowerhalf_s *lower)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;

  sninfo("Resetting position to zero\n");
  DEBUGASSERT(lower && priv->inuse);

  /* Reset the counter to zero */

  ls_putreg32(priv, LS_GTIM_CNT_OFFSET, 0);
  return OK;
}

/****************************************************************************
 * Name: ls_setindex
 *
 * Description:
 *   Set the index pin position
 *
 ****************************************************************************/

static int ls_setindex(struct qe_lowerhalf_s *lower, uint32_t pos)
{
  return -ENOTTY;
}

/****************************************************************************
 * Name: ls_ioctl
 *
 * Description:
 *   Lower-half logic may support platform-specific ioctl commands
 *
 ****************************************************************************/

static int ls_ioctl(struct qe_lowerhalf_s *lower, int cmd,
                    unsigned long arg)
{
  /* No ioctl commands supported */

  /* TODO add an IOCTL to control the encoder pulse count prescaler */

  return -ENOTTY;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_qeinitialize
 *
 * Description:
 *   Initialize a quadrature encoder interface.  This function must be
 *   called from board-specific logic.
 *
 * Input Parameters:
 *   devpath - The full path to the driver to register. E.g., "/dev/qe0"
 *   tim     - The timer number to used.  'tim' must be an element of {1,2}
 *
 * Returned Value:
 *   Zero on success; A negated errno value is returned on failure.
 *
 ****************************************************************************/

int ls_qeinitialize(const char *devpath, int tim)
{
  struct ls_lowerhalf_s *priv;
  int ret;

  /* Find the pre-allocated timer state structure corresponding to this
   * timer
   */

  priv = ls_tim2lower(tim);
  if (!priv)
    {
      snerr("ERROR: TIM%d support not configured\n", tim);
      return -ENXIO;
    }

  /* Make sure that it is available */

  if (priv->inuse)
    {
      snerr("ERROR: TIM%d is in-use\n", tim);
      return -EBUSY;
    }

  /* Register the upper-half driver */

  ret = qe_register(devpath, (struct qe_lowerhalf_s *)priv);
  if (ret < 0)
    {
      snerr("ERROR: qe_register failed: %d\n", ret);
      return ret;
    }

  /* Make sure that the timer is in the shutdown state */

  ls_shutdown((struct qe_lowerhalf_s *)priv);

  /* The driver is now in-use */

  priv->inuse = true;
  return OK;
}

#endif /* CONFIG_SENSORS_QENCODER */
