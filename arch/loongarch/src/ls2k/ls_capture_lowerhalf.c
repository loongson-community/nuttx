/****************************************************************************
 * arch/loongarch/src/ls2k/ls_capture_lowerhalf.c
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

#include <sys/types.h>

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <nuttx/irq.h>
#include <nuttx/timers/capture.h>

#include <arch/board/board.h>

#include "ls_capture.h"

#if defined(CONFIG_LS_CAP)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LS_TIM1_RES   32
#define LS_TIM2_RES   32

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure provides the private representation of the "lower-half"
 * driver state structure.  This structure must be cast-compatible with the
 * cap_lowerhalf_s structure.
 */

struct ls_lowerhalf_s
{
  const struct cap_ops_s *ops;       /* Lower half operations */
  struct ls_cap_dev_s    *cap;       /* LS capture driver */
  bool                   started;    /* True: Timer has been started */
  const uint8_t          resolution; /* Number of bits in the timer */
  uint8_t                channel;    /* pwm input channel */
  uint32_t               clock;      /* Timer clock frequency */
  uint8_t                duty;       /* Result pwm frequency */
  uint32_t               freq;       /* Result pwm frequency */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int ls_cap_handler(int irq, void * context, void * arg);

/* "Lower half" driver methods **********************************************/

static int ls_start(struct cap_lowerhalf_s *lower);
static int ls_stop(struct cap_lowerhalf_s *lower);
static int ls_getduty(struct cap_lowerhalf_s *lower, uint8_t *duty);
static int ls_getfreq(struct cap_lowerhalf_s *lower, uint32_t *freq);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* "Lower half" driver methods */

static const struct cap_ops_s g_cap_ops =
{
  .start       = ls_start,
  .stop        = ls_stop,
  .getduty     = ls_getduty,
  .getfreq     = ls_getfreq,
};

#ifdef CONFIG_LS_TIM1_CAP
static struct ls_lowerhalf_s g_cap1_lowerhalf =
{
  .ops         = &g_cap_ops,
  .resolution  = LS_TIM1_RES,
  .channel     = CONFIG_LS_TIM1_CHANNEL,
  .clock       = CONFIG_LS_TIM1_CLOCK,
};
#endif

#ifdef CONFIG_LS_TIM2_CAP
static struct ls_lowerhalf_s g_cap2_lowerhalf =
{
  .ops         = &g_cap_ops,
  .resolution  = LS_TIM2_RES,
  .channel     = CONFIG_LS_TIM2_CHANNEL,
  .clock       = CONFIG_LS_TIM2_CLOCK,
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_cap_handler
 *
 * Description:
 *   timer interrupt handler
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

static int ls_cap_handler(int irq, void * context, void * arg)
{
  struct ls_lowerhalf_s *lower = (struct ls_lowerhalf_s *) arg;
  uint8_t ch = 0x3 & lower->channel;
  int period = 0;
  int flags = 0;

  flags = (int)LS_CAP_GETFLAGS(lower->cap) ;

  LS_CAP_ACKFLAGS(lower->cap, flags);

  period = LS_CAP_GETCAPTURE(lower->cap, ch);

  if (period != 0)
    {
      lower->duty = (100 * LS_CAP_GETCAPTURE(lower->cap, 0x3 & (~ch))) /
                    period;
    }
  else
    {
      lower->duty = 0;
    }

  lower->freq = lower->clock / period;

  return OK;
}

/****************************************************************************
 * Name: ls_start
 *
 * Description:
 *   Start the timer, resetting the time to the current timeout,
 *
 * Input Parameters:
 *   lower - A pointer the publicly visible representation of the
 *           "lower-half" driver state structure.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int ls_start(struct cap_lowerhalf_s *lower)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;
  int flags = 0;
  uint32_t maxtimeout = (1 << priv->resolution) - 1;

  if (priv->started)
    {
      /* Return EBUSY to indicate that the timer was already running */

      return -EBUSY;
    }

  switch (priv->channel)
    {
      case 1:
        LS_CAP_SETSMC(priv->cap, LS_CAP_SMS_RST |
                      LS_CAP_TS_TI1FP1 |
                      LS_CAP_MSM_MASK);

        LS_CAP_SETCLOCK(priv->cap, priv->clock, maxtimeout);

        LS_CAP_SETCHANNEL(priv->cap, 1,
                          LS_CAP_EDGE_RISING |
                          LS_CAP_MAPPED_TI1);
        LS_CAP_SETCHANNEL(priv->cap, 2,
                          LS_CAP_EDGE_FALLING |
                          LS_CAP_MAPPED_TI2);

        flags = (int)LS_CAP_GETFLAGS(priv->cap);
        LS_CAP_ACKFLAGS(priv->cap, flags);

        LS_CAP_SETISR(priv->cap, ls_cap_handler, priv);
        LS_CAP_ENABLEINT(priv->cap, LS_CAP_FLAG_IRQ_CH_1, true);

        priv->started = true;
        break;

      case 2:
        LS_CAP_SETSMC(priv->cap, LS_CAP_SMS_RST |
                      LS_CAP_TS_TI2FP2 |
                      LS_CAP_MSM_MASK);

        LS_CAP_SETCLOCK(priv->cap, priv->clock, maxtimeout);

        LS_CAP_SETCHANNEL(priv->cap, 2,
                          LS_CAP_EDGE_RISING |
                          LS_CAP_MAPPED_TI1);
        LS_CAP_SETCHANNEL(priv->cap, 1,
                          LS_CAP_EDGE_FALLING |
                          LS_CAP_MAPPED_TI2);

        flags = (int)LS_CAP_GETFLAGS(priv->cap);
        LS_CAP_ACKFLAGS(priv->cap, flags);

        LS_CAP_SETISR(priv->cap, ls_cap_handler, priv);
        LS_CAP_ENABLEINT(priv->cap, LS_CAP_FLAG_IRQ_CH_2, true);

        priv->started = true;
        break;

      default:
        return ERROR;
    }

  return OK;
}

/****************************************************************************
 * Name: ls_stop
 *
 * Description:
 *   Stop the capture
 *
 * Input Parameters:
 *   lower - A pointer the publicly visible representation of the
 *           "lower-half" driver state structure.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int ls_stop(struct cap_lowerhalf_s *lower)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;

  if (priv->started)
    {
      LS_CAP_SETCHANNEL(priv->cap, LS_CAP_FLAG_IRQ_COUNTER,
                        LS_CAP_EDGE_DISABLED);
      switch (priv->channel)
        {
          case 1:
            LS_CAP_ENABLEINT(priv->cap, LS_CAP_FLAG_IRQ_CH_1, false);
            break;

          case 2:
            LS_CAP_ENABLEINT(priv->cap, LS_CAP_FLAG_IRQ_CH_2, false);
            break;

          default:
            return ERROR;
        }

      LS_CAP_SETISR(priv->cap, NULL, NULL);
      priv->started = false;
      return OK;
    }

  /* Return ENODEV to indicate that the timer was not running */

  return -ENODEV;
}

/****************************************************************************
 * Name: ls_getduty
 *
 * Description:
 *   get result duty
 *
 * Input Parameters:
 *   lower - A pointer the publicly visible representation of the
 *             "lower-half" driver state structure.
 *   duty  - DutyCycle * 100.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int ls_getduty(struct cap_lowerhalf_s *lower, uint8_t *duty)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;

  irqstate_t flags = enter_critical_section();

  *duty = priv->duty;

  leave_critical_section(flags);

  return OK;
}

/****************************************************************************
 * Name: ls_getfreq
 *
 * Description:
 *   get result freq
 *
 * Input Parameters:
 *   lower - A pointer the publicly visible representation of the
 *             "lower-half" driver state structure.
 *   freq  - Frequency in Hz.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int ls_getfreq(struct cap_lowerhalf_s *lower, uint32_t *freq)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;

  irqstate_t flags = enter_critical_section();

  *freq = priv->freq;

  leave_critical_section(flags);

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_cap_initialize
 *
 * Description:
 *   Initialize one timer for use with the upper_level capture driver.
 *
 * Input Parameters:
 *   timer - A number identifying the timer use.  The number of valid timer
 *     IDs varies with the LS MCU and MCU family but is somewhere in
 *     the range of {1,..,5 8,...,14}.
 *
 * Returned Value:
 *   On success, a pointer to the LS lower half capture driver returned.
 *   NULL is returned on any failure.
 *
 ****************************************************************************/

struct cap_lowerhalf_s *ls_cap_initialize(int timer)
{
  struct ls_lowerhalf_s *lower = NULL;

  switch (timer)
    {
#ifdef CONFIG_LS_TIM1_CAP
      case 1:
        lower = &g_cap1_lowerhalf;
        break;
#endif
#ifdef CONFIG_LS_TIM2_CAP
      case 2:
        lower = &g_cap2_lowerhalf;
        break;
#endif
      default:
        {
          lower = NULL;
          goto errout;
        }
    }

  /* Initialize the elements of lower half state structure */

  lower->started  = false;
  lower->cap      = ls_cap_init(timer);

  if (lower->cap == NULL)
    {
      lower = NULL;
    }

errout:
  return (struct cap_lowerhalf_s *)lower;
}

#endif /* CONFIG_LS_CAP */
