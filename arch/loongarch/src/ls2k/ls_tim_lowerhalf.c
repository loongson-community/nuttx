/****************************************************************************
 * arch/loongarch/src/ls2k/ls_tim_lowerhalf.c
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2015 Wail Khemir. All rights reserved.
 * SPDX-FileCopyrightText: 2015 Omni Hoverboards Inc. All rights reserved.
 * SPDX-FileContributor: Wail Khemir <khemirwail@gmail.com>
 * SPDX-FileContributor: Paul Alexander Patience <paul-a.patience@polymtl.ca>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
#include <nuttx/timers/timer.h>

#include <arch/board/board.h>

#include "ls_tim.h"

#if defined(CONFIG_TIMER) && \
    (defined(CONFIG_LS_TIM1) || defined(CONFIG_LS_TIM2)  || \
     defined(CONFIG_LS_TIM6))

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LS_TIM1_RES   32
#define LS_TIM2_RES   32
#define LS_TIM6_RES   32

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure provides the private representation of the "lower-half"
 * driver state structure.  This structure must be cast-compatible with the
 * timer_lowerhalf_s structure.
 */

struct ls_lowerhalf_s
{
  const struct timer_ops_s *ops;        /* Lower half operations */
  struct ls_tim_dev_s      *tim;        /* ls timer driver */
  tccb_t                    callback;   /* Current user interrupt callback */
  void                     *arg;        /* Argument passed to upper half callback */
  bool                      started;    /* True: Timer has been started */
  const uint8_t             resolution; /* Number of bits in the timer (16 or 32 bits) */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int ls_timer_handler(int irq, void * context, void * arg);

/* "Lower half" driver methods **********************************************/

static int ls_start(struct timer_lowerhalf_s *lower);
static int ls_stop(struct timer_lowerhalf_s *lower);
static int ls_settimeout(struct timer_lowerhalf_s *lower,
                         uint32_t timeout);
static void ls_setcallback(struct timer_lowerhalf_s *lower,
                           tccb_t callback, void *arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* "Lower half" driver methods */

static const struct timer_ops_s g_timer_ops =
{
  .start       = ls_start,
  .stop        = ls_stop,
  .getstatus   = NULL,
  .settimeout  = ls_settimeout,
  .setcallback = ls_setcallback,
  .ioctl       = NULL,
};

#ifdef CONFIG_LS_TIM1
static struct ls_lowerhalf_s g_tim1_lowerhalf =
{
  .ops         = &g_timer_ops,
  .resolution  = LS_TIM1_RES,
};
#endif

#ifdef CONFIG_LS_TIM2
static struct ls_lowerhalf_s g_tim2_lowerhalf =
{
  .ops         = &g_timer_ops,
  .resolution  = LS_TIM2_RES,
};
#endif

#ifdef CONFIG_LS_TIM6
static struct ls_lowerhalf_s g_tim6_lowerhalf =
{
  .ops         = &g_timer_ops,
  .resolution  = LS_TIM6_RES,
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_timer_handler
 *
 * Description:
 *   timer interrupt handler
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

static int ls_timer_handler(int irq, void * context, void * arg)
{
  struct ls_lowerhalf_s *lower = (struct ls_lowerhalf_s *) arg;
  uint32_t next_interval_us = 0;

  LS_TIM_ACKINT(lower->tim, ATIM_DIER_UIE);

  if (lower->callback(&next_interval_us, lower->arg))
    {
      if (next_interval_us > 0)
        {
          LS_TIM_SETPERIOD(lower->tim, next_interval_us);
        }
    }
  else
    {
      ls_stop((struct timer_lowerhalf_s *)lower);
    }

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

static int ls_start(struct timer_lowerhalf_s *lower)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;

  if (!priv->started)
    {
      LS_TIM_SETMODE(priv->tim, LS_TIM_MODE_UP);

      if (priv->callback != NULL)
        {
          LS_TIM_SETISR(priv->tim, ls_timer_handler, priv, 0);
          LS_TIM_ENABLEINT(priv->tim, ATIM_DIER_UIE);
        }

      priv->started = true;
      return OK;
    }

  /* Return EBUSY to indicate that the timer was already running */

  return -EBUSY;
}

/****************************************************************************
 * Name: ls_stop
 *
 * Description:
 *   Stop the timer
 *
 * Input Parameters:
 *   lower - A pointer the publicly visible representation of the
 *           "lower-half" driver state structure.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int ls_stop(struct timer_lowerhalf_s *lower)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;

  if (priv->started)
    {
      LS_TIM_SETMODE(priv->tim, LS_TIM_MODE_DISABLED);
      LS_TIM_DISABLEINT(priv->tim, ATIM_DIER_UIE);
      LS_TIM_SETISR(priv->tim, NULL, NULL, 0);
      priv->started = false;
      return OK;
    }

  /* Return ENODEV to indicate that the timer was not running */

  return -ENODEV;
}

/****************************************************************************
 * Name: ls_settimeout
 *
 * Description:
 *   Set a new timeout value (and reset the timer)
 *
 * Input Parameters:
 *   lower   - A pointer the publicly visible representation of the
 *             "lower-half" driver state structure.
 *   timeout - The new timeout value in microseconds.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int ls_settimeout(struct timer_lowerhalf_s *lower,
                         uint32_t timeout)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;
  uint64_t maxtimeout;

  if (priv->started)
    {
      return -EPERM;
    }

  maxtimeout = (1 << priv->resolution) - 1;
  if (timeout > maxtimeout)
    {
      uint64_t freq = (maxtimeout * 1000000) / timeout;
      LS_TIM_SETCLOCK(priv->tim, freq);
      LS_TIM_SETPERIOD(priv->tim, maxtimeout);
    }
  else
    {
      LS_TIM_SETCLOCK(priv->tim, 1000000);
      LS_TIM_SETPERIOD(priv->tim, timeout);
    }

  return OK;
}

/****************************************************************************
 * Name: ls_setcallback
 *
 * Description:
 *   Call this user provided timeout callback.
 *
 * Input Parameters:
 *   lower    - A pointer the publicly visible representation of the
 *              "lower-half" driver state structure.
 *   callback - The new timer expiration function pointer.  If this
 *              function pointer is NULL, then the reset-on-expiration
 *              behavior is restored,
 *  arg       - Argument that will be provided in the callback.
 *
 * Returned Value:
 *   The previous timer expiration function pointer or NULL is there was
 *   no previous function pointer.
 *
 ****************************************************************************/

static void ls_setcallback(struct timer_lowerhalf_s *lower,
                           tccb_t callback, void *arg)
{
  struct ls_lowerhalf_s *priv = (struct ls_lowerhalf_s *)lower;

  irqstate_t flags = enter_critical_section();

  /* Save the new callback */

  priv->callback = callback;
  priv->arg      = arg;

  if (callback != NULL && priv->started)
    {
      LS_TIM_SETISR(priv->tim, ls_timer_handler, priv, 0);
      LS_TIM_ENABLEINT(priv->tim, ATIM_DIER_UIE);
    }
  else
    {
      LS_TIM_DISABLEINT(priv->tim, ATIM_DIER_UIE);
      LS_TIM_SETISR(priv->tim, NULL, NULL, 0);
    }

  leave_critical_section(flags);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_timer_initialize
 *
 * Description:
 *   Bind the configuration timer to a timer lower half instance and
 *   register the timer drivers at 'devpath'
 *
 * Input Parameters:
 *   devpath - The full path to the timer device.  This should be of the
 *     form /dev/timer0
 *   timer - the timer's number.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returned
 *   to indicate the nature of any failure.
 *
 ****************************************************************************/

int ls_timer_initialize(const char *devpath, int timer)
{
  struct ls_lowerhalf_s *lower;

  switch (timer)
    {
#ifdef CONFIG_LS_TIM1
      case 1:
        lower = &g_tim1_lowerhalf;
        break;
#endif
#ifdef CONFIG_LS_TIM2
      case 2:
        lower = &g_tim2_lowerhalf;
        break;
#endif
#ifdef CONFIG_LS_TIM6
      case 6:
        lower = &g_tim6_lowerhalf;
        break;
#endif
      default:
        return -ENODEV;
    }

  /* Initialize the elements of lower half state structure */

  lower->started  = false;
  lower->callback = NULL;
  lower->tim      = ls_tim_init(timer);

  if (lower->tim == NULL)
    {
      return -EINVAL;
    }

  /* Register the timer driver as /dev/timerX.  The returned value from
   * timer_register is a handle that could be used with timer_unregister().
   * REVISIT: The returned handle is discard here.
   */

  void *drvr = timer_register(devpath,
                              (struct timer_lowerhalf_s *)lower);
  if (drvr == NULL)
    {
      /* The actual cause of the failure may have been a failure to allocate
       * perhaps a failure to register the timer driver (such as if the
       * 'depath' were not unique).  We know here but we return EEXIST to
       * indicate the failure (implying the non-unique devpath).
       */

      return -EEXIST;
    }

  return OK;
}

#endif /* CONFIG_TIMER */
