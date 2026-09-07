/****************************************************************************
 * arch/loongarch/src/ls2k/hardware/ls_memorymap.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_MEMORYMAP_H
#define __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_MEMORYMAP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef __ASSEMBLY__

#define UNCACHED_MEMORY_ADDR        0x8000000000000000
#define CACHED_MEMORY_ADDR          0x9000000000000000

#define PHYS_TO_UNCACHED(x)         (UNCACHED_MEMORY_ADDR | (x))
#define PHYS_TO_CACHED(x)           (CACHED_MEMORY_ADDR | (x))

#else

#define UNCACHED_MEMORY_ADDR        0x8000000000000000UL
#define CACHED_MEMORY_ADDR          0x9000000000000000UL

#define PHYS_TO_UNCACHED(x)         (UNCACHED_MEMORY_ADDR | (unsigned long)(x))
#define PHYS_TO_CACHED(x)           (CACHED_MEMORY_ADDR | (unsigned long)(x))

#endif

#define LS_DDR_BASE                 0x00000000
#define LS_DDR_SIZE                 0x10000000
#define LS_DDR_END                  (LS_DDR_BASE + LS_DDR_SIZE)

#define LS_SPIFLASH_BASE            0x1C000000
#define LS_SPIFLASH_SIZE            0x00100000

#define LS_L2CACHE_BASE             0x90000000
#define LS_L2CACHE_SIZE             0x00060000

#define LS_CHIP_CFG_BASE            PHYS_TO_UNCACHED(0x16000100)

#define LS_TSENSOR_BASE             PHYS_TO_UNCACHED(0x16001500)

#define LS_UART0_BASE               PHYS_TO_UNCACHED(0x16100000)
#define LS_UART1_BASE               PHYS_TO_UNCACHED(0x16100400)
#define LS_UART2_BASE               PHYS_TO_UNCACHED(0x16100800)
#define LS_UART3_BASE               PHYS_TO_UNCACHED(0x16100c00)
#define LS_UART4_BASE               PHYS_TO_UNCACHED(0x16101000)
#define LS_UART5_BASE               PHYS_TO_UNCACHED(0x16101400)
#define LS_UART6_BASE               PHYS_TO_UNCACHED(0x16101800)
#define LS_UART7_BASE               PHYS_TO_UNCACHED(0x16101c00)
#define LS_UART8_BASE               PHYS_TO_UNCACHED(0x16102000)
#define LS_UART9_BASE               PHYS_TO_UNCACHED(0x16102400)

#define LS_SPI0_BASE                0x16010000
#define LS_SPI1_BASE                0x16018000
#define LS_SPI2_BASE                0x1601c000
#define LS_SPI3_BASE                0x1601e000

#define LS_I2C0_BASE                PHYS_TO_UNCACHED(0x16108000)
#define LS_I2C1_BASE                PHYS_TO_UNCACHED(0x16109000)
#define LS_I2C2_BASE                PHYS_TO_UNCACHED(0x1610a000)
#define LS_I2C3_BASE                PHYS_TO_UNCACHED(0x1610b000)

#define LS_GPIO_BASE                PHYS_TO_UNCACHED(0x16104000)

#define LS_HPET0_BASE               PHYS_TO_UNCACHED(0x16120000)
#define LS_HPET1_BASE               PHYS_TO_UNCACHED(0x16121000)
#define LS_HPET2_BASE               PHYS_TO_UNCACHED(0x16122000)
#define LS_HPET3_BASE               PHYS_TO_UNCACHED(0x16123000)

#define LS_TIM1_BASE                PHYS_TO_UNCACHED(0x16118000)
#define LS_TIM2_BASE                PHYS_TO_UNCACHED(0x16119000)
#define LS_TIM6_BASE                PHYS_TO_UNCACHED(0x1611a000)

#define LS_PWM0_BASE                PHYS_TO_UNCACHED(0x1611b000)
#define LS_PWM1_BASE                PHYS_TO_UNCACHED(0x1611b010)
#define LS_PWM2_BASE                PHYS_TO_UNCACHED(0x1611b020)
#define LS_PWM3_BASE                PHYS_TO_UNCACHED(0x1611b030)

#define LS_RTC_BASE                 PHYS_TO_UNCACHED(0x16128000)

#define LS_EHCI_BASE                PHYS_TO_UNCACHED(0x16080000)
#define LS_XHCI_BASE                PHYS_TO_UNCACHED(0x16088000)

#define LS_DC_BASE                  PHYS_TO_UNCACHED(0x16090000)

#define LS_SDIO0_BASE               PHYS_TO_UNCACHED(0x1ff64000)
#define LS_SDIO1_BASE               PHYS_TO_UNCACHED(0x1ff66000)

#define LS_WDT_BASE                 PHYS_TO_UNCACHED(0x16124000)

#define LS_ADC1_BASE                PHYS_TO_UNCACHED(0x1611c000)

#define LS_DMA1_BASE                PHYS_TO_UNCACHED(0x1612c000)
#define LS_DMA1_CHANNEL_BASE(n)     (LS_DMA1_BASE + 0x08 + 0x14 * (n))

#define LS_SCACHE_LOCK_WIN0_BASE    0x16000200
#define LS_SCACHE_LOCK_WIN1_BASE    0x16000208
#define LS_SCACHE_LOCK_WIN2_BASE    0x16000210
#define LS_SCACHE_LOCK_WIN3_BASE    0x16000218
#define LS_SCACHE_LOCK_WIN0_MASK    0x16000240
#define LS_SCACHE_LOCK_WIN1_MASK    0x16000248
#define LS_SCACHE_LOCK_WIN2_MASK    0x16000250
#define LS_SCACHE_LOCK_WIN3_MASK    0x16000258

#define LS_NODE_PLL_L               PHYS_TO_UNCACHED(0x16000400)
#define LS_NODE_PLL_H               PHYS_TO_UNCACHED(0x16000404)
#define LS_DDR_PLL_L                PHYS_TO_UNCACHED(0x16000408)
#define LS_DDR_PLL_H                PHYS_TO_UNCACHED(0x1600040c)
#define LS_PIX0_PLL                 PHYS_TO_UNCACHED(0x16000410)
#define LS_PIX1_PLL                 PHYS_TO_UNCACHED(0x16000414)
#define LS_FREQ_SCALE               PHYS_TO_UNCACHED(0x16000420)

#define LS_GENERAL_CFG0             0x16000100
#define LS_GENERAL_CFG1             0x16000104
#define LS_GENERAL_CFG2             0x16000108
#define LS_GENERAL_CFG3             0x1600010c
#define LS_GENERAL_CFG4             0x16000110
#define LS_GENERAL_CFG5             0x16000114

#define LS_CHIP_HPT_LO              PHYS_TO_UNCACHED(0x16000150)
#define LS_CHIP_HPT_HI              PHYS_TO_UNCACHED(0x16000154)

#define LS_PINCTRL_BASE             PHYS_TO_UNCACHED(0x16000490)
#define LS_GPIO_32_47_MULTI_CFG     0x16000498

#endif /* __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_MEMORYMAP_H */
