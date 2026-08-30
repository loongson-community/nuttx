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
#include <nuttx/spi/spi.h>

#include <ls.h>

#include <arch/board/board.h>

#include "hummingbird-ls2k0300.h"

#include <nuttx/board.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

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

#ifdef CONFIG_LS_PWM0
  ret = ls_pwm_initialize(0);
  if (ret < 0)
    {
      serr("ERROR: ls_pwm_initialize failed for PWM0: %d\n", ret);
    }
#endif

#ifdef CONFIG_LS_PWM1
  ret = ls_pwm_initialize(1);
  if (ret < 0)
    {
      serr("ERROR: ls_pwm_initialize failed for PWM1: %d\n", ret);
    }
#endif

#ifdef CONFIG_LS_PWM2
  ret = ls_pwm_initialize(2);
  if (ret < 0)
    {
      serr("ERROR: ls_pwm_initialize failed for PWM2: %d\n", ret);
    }
#endif

#ifdef CONFIG_LS_PWM3
  ret = ls_pwm_initialize(3);
  if (ret < 0)
    {
      serr("ERROR: ls_pwm_initialize failed for PWM3: %d\n", ret);
    }
#endif

#ifdef CONFIG_LS_SPI0
  struct spi_dev_s *spi0 = ls_spiflash_initialize(0);
  if (spi0 == NULL)
    {
      serr("ERROR: ls_spiflash_initialize failed for SPI0\n");
    }
#endif

#ifdef CONFIG_LS_SPI1
  struct spi_dev_s *spi1 = ls_spiflash_initialize(1);
  if (spi1 == NULL)
    {
      serr("ERROR: ls_spiflash_initialize failed for SPI1\n");
    }
#endif

#ifdef CONFIG_LS_SPIIO0
  struct spi_dev_s *spiio0 = ls_spiio_initialize(0);
  if (spiio0 == NULL)
    {
      serr("ERROR: ls_spiio_initialize failed for SPIIO0\n");
    }
#endif

#ifdef CONFIG_LS_SPIIO1
  struct spi_dev_s *spiio1 = ls_spiio_initialize(1);
  if (spiio1 == NULL)
    {
      serr("ERROR: ls_spiio_initialize failed for SPIIO1\n");
    }
#endif

  return ret;
}
