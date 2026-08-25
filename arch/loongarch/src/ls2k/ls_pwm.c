/****************************************************************************
 * arch/loongarch/src/ls2k/ls_pwm.c
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
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>
#include <fixedmath.h>

#include <nuttx/arch.h>
#include <nuttx/kmalloc.h>
#include <nuttx/timers/pwm.h>

#include <arch/board/board.h>

#include "loongarch_internal.h"
#include "hardware/ls_memorymap.h"
#include "hardware/ls_pwm.h"
#include "ls_gpio.h"

#if defined(CONFIG_LS_PWM)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LS_PWM_CLOCK_FREQ 200000000UL

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct ls_pwm_priv_s
{
  struct pwm_lowerhalf_s lower;
  uintptr_t base;
  uint32_t clock_freq;
  uint32_t low_buffer;
  uint32_t full_buffer;
  uint32_t pwm_pin;
  bool initialized;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int ls_pwm_setup(struct pwm_lowerhalf_s *dev);
static int ls_pwm_shutdown(struct pwm_lowerhalf_s *dev);
static int ls_pwm_start(struct pwm_lowerhalf_s *dev,
                        const struct pwm_info_s *info);
static int ls_pwm_stop(struct pwm_lowerhalf_s *dev);
static int ls_pwm_ioctl(struct pwm_lowerhalf_s *dev, int cmd,
                        unsigned long arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct pwm_ops_s g_pwm_ops =
{
  ls_pwm_setup,
  ls_pwm_shutdown,
  ls_pwm_start,
  ls_pwm_stop,
  ls_pwm_ioctl
};

static struct ls_pwm_priv_s g_pwm_priv[4] =
{
  {
    .lower =
    {
      .ops = &g_pwm_ops
    },

    .base = LS_PWM0_BASE,
    .pwm_pin = GPIO_PWM0,
    .clock_freq = LS_PWM_CLOCK_FREQ,
  },

  {
    .lower =
    {
      .ops = &g_pwm_ops
    },

    .base = LS_PWM1_BASE,
    .pwm_pin = GPIO_PWM1,
    .clock_freq = LS_PWM_CLOCK_FREQ,
  },

  {
    .lower =
    {
      .ops = &g_pwm_ops
    },

    .base = LS_PWM2_BASE,
    .pwm_pin = GPIO_PWM2,
    .clock_freq = LS_PWM_CLOCK_FREQ,
  },

  {
    .lower =
    {
      .ops = &g_pwm_ops
    },

    .base = LS_PWM3_BASE,
    .pwm_pin = GPIO_PWM3,
    .clock_freq = LS_PWM_CLOCK_FREQ,
  },
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t pwm_getreg(struct ls_pwm_priv_s *priv, uint32_t offset)
{
  return getreg32(priv->base + offset);
}

static void pwm_putreg(struct ls_pwm_priv_s *priv, uint32_t offset,
                       uint32_t value)
{
  putreg32(value, priv->base + offset);
}

static int ls_pwm_setup(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwm_priv_s *priv = (struct ls_pwm_priv_s *)dev;
  uint32_t regval;

  regval = pwm_getreg(priv, LS_PWM_CTRL);
  regval &= ~PWM_CTRL_EN;
  pwm_putreg(priv, LS_PWM_CTRL, regval);

  regval = pwm_getreg(priv, LS_PWM_CTRL);
  regval |= PWM_CTRL_RST;
  pwm_putreg(priv, LS_PWM_CTRL, regval);
  regval &= ~PWM_CTRL_RST;
  pwm_putreg(priv, LS_PWM_CTRL, regval);

  priv->initialized = true;
  return OK;
}

static int ls_pwm_shutdown(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwm_priv_s *priv = (struct ls_pwm_priv_s *)dev;
  uint32_t regval;

  pwm_putreg(priv, LS_PWM_LOW_BUFFER, 1);
  if (!priv->full_buffer)
    {
      pwm_putreg(priv, LS_PWM_FULL_BUFFER, 10000);
    }

  regval = pwm_getreg(priv, LS_PWM_CTRL);
  regval |= PWM_CTRL_RST;
  pwm_putreg(priv, LS_PWM_CTRL, regval);
  regval &= ~PWM_CTRL_RST;
  pwm_putreg(priv, LS_PWM_CTRL, regval);

  regval = pwm_getreg(priv, LS_PWM_CTRL);
  regval &= ~PWM_CTRL_EN;
  pwm_putreg(priv, LS_PWM_CTRL, regval);

  priv->initialized = false;
  return OK;
}

static int ls_pwm_start(struct pwm_lowerhalf_s *dev,
                        const struct pwm_info_s *info)
{
  struct ls_pwm_priv_s *priv = (struct ls_pwm_priv_s *)dev;
  uint32_t regval;
  uint64_t val;

  val = (uint64_t)priv->clock_freq * info->frequency;
  val = val / 1000000UL;
  if (val < 1)
    {
      val = 1;
    }

  priv->full_buffer = (uint32_t)val;

  val = (uint64_t)priv->full_buffer * (uint64_t)info->channels[0].duty;
  val = val / 65536UL;
  if (val < 1)
    {
      val = 1;
    }

  priv->low_buffer = (uint32_t)val;

  pwm_putreg(priv, LS_PWM_LOW_BUFFER, priv->low_buffer);
  pwm_putreg(priv, LS_PWM_FULL_BUFFER, priv->full_buffer);

  regval = pwm_getreg(priv, LS_PWM_CTRL);
  regval |= PWM_CTRL_EN | PWM_CTRL_OE;
  pwm_putreg(priv, LS_PWM_CTRL, regval);

  return OK;
}

static int ls_pwm_stop(struct pwm_lowerhalf_s *dev)
{
  struct ls_pwm_priv_s *priv = (struct ls_pwm_priv_s *)dev;
  uint32_t regval;

  pwm_putreg(priv, LS_PWM_LOW_BUFFER, 1);
  if (!priv->full_buffer)
    {
      pwm_putreg(priv, LS_PWM_FULL_BUFFER, 10000);
    }

  regval = pwm_getreg(priv, LS_PWM_CTRL);
  regval |= PWM_CTRL_RST;
  pwm_putreg(priv, LS_PWM_CTRL, regval);
  regval &= ~PWM_CTRL_RST;
  pwm_putreg(priv, LS_PWM_CTRL, regval);

  regval = pwm_getreg(priv, LS_PWM_CTRL);
  regval &= ~PWM_CTRL_EN;
  pwm_putreg(priv, LS_PWM_CTRL, regval);

  return OK;
}

static int ls_pwm_ioctl(struct pwm_lowerhalf_s *dev, int cmd,
                        unsigned long arg)
{
  struct ls_pwm_priv_s *priv =
      (struct ls_pwm_priv_s *)dev;
  uint32_t regval;

  switch (cmd)
    {
      case PWMIOC_START:
        return ls_pwm_start(dev, (const struct pwm_info_s *)arg);

      case PWMIOC_STOP:
        return ls_pwm_stop(dev);

      default:
        break;
    }

  switch (cmd)
    {
      case 0x2001:
        regval = pwm_getreg(priv, LS_PWM_CTRL);
        regval |= PWM_CTRL_INVERT;
        pwm_putreg(priv, LS_PWM_CTRL, regval);
        break;

      case 0x2002:
        regval = pwm_getreg(priv, LS_PWM_CTRL);
        regval &= ~PWM_CTRL_INVERT;
        pwm_putreg(priv, LS_PWM_CTRL, regval);
        break;

      default:
        return -ENOTTY;
    }

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int ls_pwm_initialize(int port)
{
  char path[16];
  int ret;

  if (port < 0 || port > 3)
    {
      return -EINVAL;
    }

  snprintf(path, sizeof(path), "/dev/pwm%d", port);

  struct ls_pwm_priv_s *priv = &g_pwm_priv[port];

  /* Configure pins */

  if (ls_configgpio(priv->pwm_pin) < 0)
    {
      return ERROR;
    }

  ret = pwm_register(path, &priv->lower);
  if (ret < 0)
    {
      pwmerr("ERROR: pwm_register failed: %d\n", ret);
      return ret;
    }

  return OK;
}

#endif
