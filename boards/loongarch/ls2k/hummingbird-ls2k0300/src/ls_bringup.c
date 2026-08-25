/****************************************************************************
 * boards/loongarch/ls2k/hummingbird-ls2k0300/src/ls_bringup.c
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

#include <stdio.h>
#include <sys/param.h>
#include <syslog.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/arch.h>

#include <ls.h>

#include <arch/board/board.h>

#include "hummingbird-ls2k0300.h"

#include <nuttx/board.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#if defined(CONFIG_I2C) && defined(CONFIG_SYSTEM_I2CTOOL)
static void ls_i2c_register(int bus)
{
  struct i2c_master_s *i2c;
  int ret;

  i2c = ls_i2cbus_initialize(bus);
  if (i2c == NULL)
    {
      _err("ERROR: Failed to get I2C%d interface\n", bus);
    }
  else
    {
      ret = i2c_register(i2c, bus);
      if (ret < 0)
        {
          _err("ERROR: Failed to register I2C%d driver: %d\n", bus, ret);
          ls_i2cbus_uninitialize(i2c);
        }
    }
}
#endif

#if defined(CONFIG_I2C) && defined(CONFIG_SYSTEM_I2CTOOL)
static void ls_i2ctool(void)
{
#ifdef CONFIG_LS_I2C0
  ls_i2c_register(0);
#endif
#ifdef CONFIG_LS_I2C1
  ls_i2c_register(1);
#endif
#ifdef CONFIG_LS_I2C2
  ls_i2c_register(2);
#endif
#ifdef CONFIG_LS_I2C3
  ls_i2c_register(3);
#endif
}
#else
#  define ls_i2ctool()
#endif

/****************************************************************************
 * Name: ls_bringup
 *
 * Description:
 *   Perform architecture-specific initialization
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y :
 *     Called from board_late_initialize().
 *
 ****************************************************************************/

int ls_bringup(void)
{
  int ret = OK;

  /* Register I2C drivers on behalf of the I2C tool */

  ls_i2ctool();

#ifdef CONFIG_LS_GPIO
  ls_gpioinit();

  /* Configure UART pins */

#  ifdef CONFIG_16550_UART0
  ls_configgpio(GPIO_UART0_RX);
  ls_configgpio(GPIO_UART0_TX);
#  endif
#  ifdef CONFIG_16550_UART1
  ls_configgpio(GPIO_UART1_RX);
  ls_configgpio(GPIO_UART1_TX);
#  endif
#endif

#ifdef CONFIG_FS_PROCFS
  ret = nx_mount(NULL, CONFIG_NSH_PROC_MOUNTPOINT, "procfs", 0, NULL);
  if (ret < 0)
    {
      serr("ERROR: Failed to mount procfs at %s: %d\n", "/proc", ret);
    }
#endif

#ifdef CONFIG_LS_WDT
  ret = ls_wdt_initialize();
  if (ret < 0)
    {
      serr("ERROR: ls_wdt_initialize failed: %d\n", ret);
    }
#endif

  return ret;
}
