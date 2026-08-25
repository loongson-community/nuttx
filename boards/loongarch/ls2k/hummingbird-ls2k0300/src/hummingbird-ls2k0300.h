/****************************************************************************
 * boards/loongarch/ls2k/hummingbird-ls2k0300/src/hummingbird-ls2k0300.h
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

#ifndef __BOARDS_LOONGARCH_LS2K_HUMMINGBIRD_LS2K0300_SRC_HUMMINGBIRD_LS2K0300_H
#define __BOARDS_LOONGARCH_LS2K_HUMMINGBIRD_LS2K0300_SRC_HUMMINGBIRD_LS2K0300_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>
#include <stdint.h>
#include <arch/chip/chip.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* LS2K0300 Humminhbird GPIOs ***********************************************/

/* LEDs */

#define GPIO_LED1     (GPIO_IO_OUT | GPIO_PIN(85))

/* Button definitions *******************************************************/

/* The LS2K0300 Humminhbird supports two buttons; only one button is
 * controllable by software:
 *
 *   B1 USER: user button connected to GPIO83
 *   B2 RESET: pushbutton connected to NRST is used to RESET
 *             the LS2K0300.
 *
 */

#define GPIO_BTN_USER   (GPIO_IO_IN | GPIO_PIN(83))

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************
 * Public Function Prototypes
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

int ls_bringup(void);

#ifdef CONFIG_LS_GPIO
extern void ls_gpioinit(void);
#endif

#ifdef CONFIG_LS_ADC
struct adc_dev_s *ls_adcinitialize(int intf, const uint8_t *chanlist,
                                   int channels);
#endif

#ifdef CONFIG_LS_I2C
extern struct i2c_master_s *ls_i2cbus_initialize(int port);
#endif

#ifdef CONFIG_LS_PWM
extern int ls_pwm_initialize(int port);
#endif

#ifdef CONFIG_LS_SPI
extern struct spi_dev_s *ls_spiflash_initialize(int port);
#endif

#ifdef CONFIG_LS_SPIIO
extern struct spi_dev_s *ls_spiio_initialize(int port);
#endif

#ifdef CONFIG_LS_THERMAL
extern int ls_thermal_initialize(void);
#endif

#ifdef CONFIG_LS_TIM
extern int ls_timer_initialize(const char *devpath, int timer);
#endif

#ifdef CONFIG_LS_WDT
extern int ls_wdt_initialize(void);
#endif

#endif /* __ASSEMBLY__ */
#endif /* __BOARDS_LOONGARCH_LS2K_HUMMINGBIRD_LS2K0300_SRC_HUMMINGBIRD_LS2K0300_H */
