/****************************************************************************
 * arch/loongarch/include/ls2k/ls2k030x_irq.h
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

/* This file should never be included directly but, rather,
 * only indirectly through nuttx/irq.h
 */

#ifndef __ARCH_LOONGARCH_INCLUDE_LS2K_LS2K030x_IRQ_H
#define __ARCH_LOONGARCH_INCLUDE_LS2K_LS2K030x_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/irq.h>

/****************************************************************************
 * Pre-processor Prototypes
 ****************************************************************************/

#define LS_IRQ_PERI_START       (LOONGARCH_MAX_IRQ + 1)

#define LS_IRQ_EIOINTC_START    LS_IRQ_PERI_START
#define LS_IRQ_EIOINTC_COUNT    128

#define LS_IRQ_EIOINTC(n)       (LS_IRQ_EIOINTC_START + (n))

#define LS_IRQ_UART0            LS_IRQ_EIOINTC(0)
#define LS_IRQ_UART1            LS_IRQ_EIOINTC(1)
#define LS_IRQ_UART2            LS_IRQ_EIOINTC(2)
#define LS_IRQ_UART3            LS_IRQ_EIOINTC(3)
#define LS_IRQ_UART4            LS_IRQ_EIOINTC(4)
#define LS_IRQ_UART5            LS_IRQ_EIOINTC(5)
#define LS_IRQ_UART6            LS_IRQ_EIOINTC(6)
#define LS_IRQ_UART7            LS_IRQ_EIOINTC(7)
#define LS_IRQ_UART8            LS_IRQ_EIOINTC(8)
#define LS_IRQ_UART9            LS_IRQ_EIOINTC(9)
#define LS_IRQ_I2C0             LS_IRQ_EIOINTC(10)
#define LS_IRQ_I2C1             LS_IRQ_EIOINTC(11)
#define LS_IRQ_I2C2             LS_IRQ_EIOINTC(12)
#define LS_IRQ_I2C3             LS_IRQ_EIOINTC(13)
#define LS_IRQ_SPI2             LS_IRQ_EIOINTC(14)
#define LS_IRQ_SPI3             LS_IRQ_EIOINTC(15)
#define LS_IRQ_CAN0_CORE        LS_IRQ_EIOINTC(16)
#define LS_IRQ_CAN0_BUF         LS_IRQ_EIOINTC(17)
#define LS_IRQ_CAN1_CORE        LS_IRQ_EIOINTC(18)
#define LS_IRQ_CAN1_BUF         LS_IRQ_EIOINTC(19)
#define LS_IRQ_CAN2_CORE        LS_IRQ_EIOINTC(20)
#define LS_IRQ_CAN2_BUF         LS_IRQ_EIOINTC(21)
#define LS_IRQ_CAN3_CORE        LS_IRQ_EIOINTC(22)
#define LS_IRQ_CAN3_BUF         LS_IRQ_EIOINTC(23)
#define LS_IRQ_I2S              LS_IRQ_EIOINTC(24)
#define LS_IRQ_TIM1             LS_IRQ_EIOINTC(25)
#define LS_IRQ_TIM2             LS_IRQ_EIOINTC(26)
#define LS_IRQ_TIM6             LS_IRQ_EIOINTC(27)
#define LS_IRQ_PWM0             LS_IRQ_EIOINTC(28)
#define LS_IRQ_PWM1             LS_IRQ_EIOINTC(29)
#define LS_IRQ_PWM2             LS_IRQ_EIOINTC(30)
#define LS_IRQ_PWM3             LS_IRQ_EIOINTC(31)
#define LS_IRQ_ADC1             LS_IRQ_EIOINTC(32)
#define LS_IRQ_HPET0_INT0       LS_IRQ_EIOINTC(33)
#define LS_IRQ_HPET0_INT1       LS_IRQ_EIOINTC(34)
#define LS_IRQ_HPET0_INT2       LS_IRQ_EIOINTC(35)
#define LS_IRQ_HPET1_INT0       LS_IRQ_EIOINTC(36)
#define LS_IRQ_HPET1_INT1       LS_IRQ_EIOINTC(37)
#define LS_IRQ_HPET1_INT2       LS_IRQ_EIOINTC(38)
#define LS_IRQ_HPET2_INT0       LS_IRQ_EIOINTC(39)
#define LS_IRQ_HPET2_INT1       LS_IRQ_EIOINTC(40)
#define LS_IRQ_HPET2_INT2       LS_IRQ_EIOINTC(41)
#define LS_IRQ_HPET3_INT0       LS_IRQ_EIOINTC(42)
#define LS_IRQ_HPET3_INT1       LS_IRQ_EIOINTC(43)
#define LS_IRQ_HPET3_INT2       LS_IRQ_EIOINTC(44)
#define LS_IRQ_DMA1CH1          LS_IRQ_EIOINTC(45)
#define LS_IRQ_DMA1CH2          LS_IRQ_EIOINTC(46)
#define LS_IRQ_DMA1CH3          LS_IRQ_EIOINTC(47)
#define LS_IRQ_DMA1CH4          LS_IRQ_EIOINTC(48)
#define LS_IRQ_DMA1CH5          LS_IRQ_EIOINTC(49)
#define LS_IRQ_DMA1CH6          LS_IRQ_EIOINTC(50)
#define LS_IRQ_DMA1CH7          LS_IRQ_EIOINTC(51)
#define LS_IRQ_DMA1CH8          LS_IRQ_EIOINTC(52)
#define LS_IRQ_SDIO0            LS_IRQ_EIOINTC(53)
#define LS_IRQ_SDIO1            LS_IRQ_EIOINTC(54)
#define LS_IRQ_SDIO0_DMA        LS_IRQ_EIOINTC(55)
#define LS_IRQ_SDIO1_DMA        LS_IRQ_EIOINTC(56)
#define LS_IRQ_ENCRYPT_DMA      LS_IRQ_EIOINTC(57)
#define LS_IRQ_AES              LS_IRQ_EIOINTC(58)
#define LS_IRQ_DES              LS_IRQ_EIOINTC(59)
#define LS_IRQ_SM3              LS_IRQ_EIOINTC(60)
#define LS_IRQ_SM4              LS_IRQ_EIOINTC(61)
#define LS_IRQ_RTC_INT0         LS_IRQ_EIOINTC(62)
#define LS_IRQ_RTC_INT1         LS_IRQ_EIOINTC(63)
#define LS_IRQ_RTC_INT2         LS_IRQ_EIOINTC(64)
#define LS_IRQ_TOY_INT0         LS_IRQ_EIOINTC(65)
#define LS_IRQ_TOY_INT1         LS_IRQ_EIOINTC(66)
#define LS_IRQ_TOY_INT2         LS_IRQ_EIOINTC(67)
#define LS_IRQ_RTC_TICK         LS_IRQ_EIOINTC(68)
#define LS_IRQ_TOY_TICK         LS_IRQ_EIOINTC(69)
#define LS_IRQ_SPI0             LS_IRQ_EIOINTC(70)
#define LS_IRQ_SPI1             LS_IRQ_EIOINTC(71)
#define LS_IRQ_EHCI             LS_IRQ_EIOINTC(72)
#define LS_IRQ_OHCI             LS_IRQ_EIOINTC(73)
#define LS_IRQ_OTG              LS_IRQ_EIOINTC(74)
#define LS_IRQ_GMAC0            LS_IRQ_EIOINTC(75)
#define LS_IRQ_GMAC1            LS_IRQ_EIOINTC(76)
#define LS_IRQ_DC               LS_IRQ_EIOINTC(77)
#define LS_IRQ_TSENSOR          LS_IRQ_EIOINTC(78)
#define LS_IRQ_GPIO_0_3         LS_IRQ_EIOINTC(79)
#define LS_IRQ_GPIO_4_7         LS_IRQ_EIOINTC(80)
#define LS_IRQ_GPIO_8_11        LS_IRQ_EIOINTC(81)
#define LS_IRQ_GPIO_12_15       LS_IRQ_EIOINTC(82)
#define LS_IRQ_GPIO_16_19       LS_IRQ_EIOINTC(83)
#define LS_IRQ_GPIO_20_23       LS_IRQ_EIOINTC(84)
#define LS_IRQ_GPIO_24_27       LS_IRQ_EIOINTC(85)
#define LS_IRQ_GPIO_28_31       LS_IRQ_EIOINTC(86)
#define LS_IRQ_GPIO_32_35       LS_IRQ_EIOINTC(87)
#define LS_IRQ_GPIO_36_39       LS_IRQ_EIOINTC(88)
#define LS_IRQ_GPIO_40_43       LS_IRQ_EIOINTC(89)
#define LS_IRQ_GPIO_44_47       LS_IRQ_EIOINTC(90)
#define LS_IRQ_GPIO_48_51       LS_IRQ_EIOINTC(91)
#define LS_IRQ_GPIO_52_55       LS_IRQ_EIOINTC(92)
#define LS_IRQ_GPIO_56_59       LS_IRQ_EIOINTC(93)
#define LS_IRQ_GPIO_60_63       LS_IRQ_EIOINTC(94)
#define LS_IRQ_GPIO_64_67       LS_IRQ_EIOINTC(95)
#define LS_IRQ_GPIO_68_71       LS_IRQ_EIOINTC(96)
#define LS_IRQ_GPIO_72_75       LS_IRQ_EIOINTC(97)
#define LS_IRQ_GPIO_76_79       LS_IRQ_EIOINTC(98)
#define LS_IRQ_GPIO_80_83       LS_IRQ_EIOINTC(99)
#define LS_IRQ_GPIO_84_87       LS_IRQ_EIOINTC(100)
#define LS_IRQ_GPIO_88_91       LS_IRQ_EIOINTC(101)
#define LS_IRQ_GPIO_92_95       LS_IRQ_EIOINTC(102)
#define LS_IRQ_GPIO_96_99       LS_IRQ_EIOINTC(103)
#define LS_IRQ_GPIO_100_103     LS_IRQ_EIOINTC(104)
#define LS_IRQ_GPIO_104_105     LS_IRQ_EIOINTC(105)
/* EIOINTC 106-110: Reserved */
#define LS_IRQ_DDR_ECC0         LS_IRQ_EIOINTC(111)
#define LS_IRQ_DDR_ECC1         LS_IRQ_EIOINTC(112)
/* EIOINTC 113-127: Reserved */

#define NR_IRQS                 (LS_IRQ_EIOINTC_START + LS_IRQ_EIOINTC_COUNT)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__
#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif

#endif /* __ARCH_LOONGARCH_INCLUDE_LS2K_LS2K030x_IRQ_H */
