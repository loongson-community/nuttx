/****************************************************************************
 * arch/loongarch/include/ls2k/chip.h
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

#ifndef __ARCH_LOONGARCH_INCLUDE_LS2K_CHIP_H
#define __ARCH_LOONGARCH_INCLUDE_LS2K_CHIP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if defined(CONFIG_ARCH_CHIP_LS2K0300) || defined(CONFIG_ARCH_CHIP_LS2K0301)

#  define LS_NATIM                    1   /* 1 advanced timers */
#  define LS_NGTIM                    1   /* 1 general timer */
#  define LS_NBTIM                    1   /* 1 basic timer with DMA */
#  define LS_NPWM                     1   /* 1 pwm controller, 4-channels */
#  define LS_NDMA                     1   /* DMA, 8-channels */
#  define LS_NSPI_FLASH               2   /* SPI0-1 */
#  define LS_NSPI_IO                  2   /* SPI2-3 */
#  define LS_NI2S                     1   /* I2S */
#  define LS_NUSART                   10  /* UART0-9 */
#  define LS_NI2C                     4   /* I2C0-3 */
#  define LS_NCAN                     4   /* CAN0-3 */
#  define LS_NSDIO                    2   /* SDIO */
#  define LS_NLCD                     1   /* LCD */
#  define LS_NUSBOTG                  1   /* USB OTG HS */
#  define LS_NGPIO                    106 /* 106 GPIOs */
#  define LS_NADC                     1   /* ADC1, 8-channels */
#  define LS_NETHERNET                2   /* GMAC0-1 */

#else
#  error "Unsupported LS chip"
#endif

#endif /* __ARCH_LOONGARCH_INCLUDE_LS_CHIP_H */
