/****************************************************************************
 * arch/loongarch/src/ls2k/ls_gpio.c
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
#include <stdbool.h>
#include <errno.h>

#include <nuttx/debug.h>

#include "loongarch_internal.h"
#include "chip.h"
#include "ls_gpio.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

const uintptr_t g_gpio_base = LS_GPIO_BASE;
const uintptr_t g_pinctrl_base = LS_PINCTRL_BASE;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void gpio_set_reg(uintptr_t base, uint32_t offset, uint8_t pin,
                         uint8_t val)
{
  putreg8(val, base + offset + pin);
}

static uint8_t gpio_get_reg(uintptr_t base, uint32_t offset, uint8_t pin)
{
  return getreg8(base + offset + pin);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Function: ls_gpioinit
 *
 * Description:
 *   Initialize GPIO. Called early in the boot sequence.
 *
 ****************************************************************************/

void ls_gpioinit(void)
{
  return;
}

/****************************************************************************
 * Name: ls_configgpio
 ****************************************************************************/

int ls_configgpio(uint32_t cfgset)
{
  uint32_t pin;
  uint32_t alt_func;
  uint32_t reg_offset;
  uint32_t bit_offset;
  uint32_t reg_val;
  uint32_t mask;

  pin = (cfgset & GPIO_PIN_MASK) >> GPIO_PIN_SHIFT;

  if (pin >= LS_GPIO_NPINS)
    {
      return -EINVAL;
    }

  /* Set the alternate function via pinctrl registers */

  alt_func = (cfgset & GPIO_ALT_MASK) >> GPIO_ALT_SHIFT;

  reg_offset = (pin / 16) * 4;
  bit_offset = (pin % 16) * 2;
  mask = 0x3 << bit_offset;

  reg_val = getreg32(g_pinctrl_base + reg_offset);
  reg_val &= ~mask;
  reg_val |= alt_func << bit_offset;
  putreg32(reg_val, g_pinctrl_base + reg_offset);

  /* Set direction */

  if (cfgset & GPIO_IO_INPUT)
    {
      gpio_set_reg(g_gpio_base, LS_GPIO_DIR_OFFSET, pin, 1);
    }
  else
    {
      gpio_set_reg(g_gpio_base, LS_GPIO_DIR_OFFSET, pin, 0);
    }

  return OK;
}

/****************************************************************************
 * Name: ls_unconfiggpio
 *
 * Description:
 *   Unconfigure a GPIO pin, resetting it to the default state (GPIO
 *   function, input direction).
 *
 ****************************************************************************/

int ls_unconfiggpio(uint32_t cfgset)
{
  cfgset &= GPIO_PIN_MASK;
  return ls_configgpio(cfgset);
}

/****************************************************************************
 * Name: ls_gpiowrite
 *
 * Description:
 *   Write one or zero to the selected GPIO pin
 *
 ****************************************************************************/

void ls_gpiowrite(uint32_t pinset, bool value)
{
  uint32_t pin;

  pin = (pinset & GPIO_PIN_MASK) >> GPIO_PIN_SHIFT;
  if (pin < LS_GPIO_NPINS)
    {
      gpio_set_reg(g_gpio_base, LS_GPIO_OUT_OFFSET, pin, value ? 1 : 0);
    }
}

/****************************************************************************
 * Name: ls_gpioread
 *
 * Description:
 *   Read one or zero from the selected GPIO pin
 *
 ****************************************************************************/

bool ls_gpioread(uint32_t pinset)
{
  uint32_t pin;

  pin = (pinset & GPIO_PIN_MASK) >> GPIO_PIN_SHIFT;
  if (pin < LS_GPIO_NPINS)
    {
      return gpio_get_reg(g_gpio_base, LS_GPIO_IN_OFFSET, pin) & 1;
    }

  return false;
}
