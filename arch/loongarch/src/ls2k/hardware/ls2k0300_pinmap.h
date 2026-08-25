/****************************************************************************
 * arch/loongarch/src/ls2k/hardware/ls2k0300_pinmap.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K0300_HARDWARE_LS2K0300_PINMAP_H
#define __ARCH_LOONGARCH_SRC_LS2K0300_HARDWARE_LS2K0300_PINMAP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "ls_gpio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* GPIO Pin Configuration Encoding:
 *
 * Each pin configuration encodes the alternate function number and the
 * pin number.  The alternate function is stored in bits [9:8] and the
 * pin number is stored in bits [7:0].
 *
 *   GPIO_ALT_M  (0x100) = Primary function   (主功能复用)
 *   GPIO_ALT_1  (0x200) = First alternate    (第一复用)
 *   GPIO_ALT_2  (0x300) = Second alternate   (第二复用)
 *
 * Extraction:
 *   PIN fn = (cfg & GPIO_ALT_MASK) >> GPIO_ALT_SHIFT
 *   PIN    = (cfg & GPIO_PIN_MASK)
 */

#define GPIO_PIN_SHIFT              (0)
#define GPIO_PIN_MASK               (0xff << GPIO_PIN_SHIFT)
#  define GPIO_PIN(n)               ((n) << GPIO_PIN_SHIFT)
#define GPIO_ALT_SHIFT              (8)
#define GPIO_ALT_MASK               (0x3 << GPIO_ALT_SHIFT)
#  define GPIO_ALT_GPIO             (0 << GPIO_ALT_SHIFT)
#  define GPIO_ALT_1                (1 << GPIO_ALT_SHIFT)
#  define GPIO_ALT_2                (2 << GPIO_ALT_SHIFT)
#  define GPIO_ALT_M                (3 << GPIO_ALT_SHIFT)
#define GPIO_IO_INPUT               (1 << 10)
#define GPIO_IO_OUTPUT              (0 << 10)
#define GPIO_INT_EN                 (1 << 11)
#define GPIO_INT_POL                (1 << 12)
#define GPIO_INT_EDGE               (1 << 13)
#define GPIO_INT_DUAL               (1 << 14)

/* Alternate Pin Functions.
 *
 * All members of the LS2K0300 family share the same pin multiplexing.
 *
 * Alternative pin selections are provided with a numeric suffix like _1, _2,
 * etc.  Drivers, however, will use the pin selection without the numeric
 * suffix.  Additional definitions are required in the board.h file.  For
 * example, if UART0_RX connects via GPIO40 on some board, then the following
 * definitions should appear in the board.h header file for that board:
 *
 *   #define GPIO_UART0_RX GPIO_UART0_RX_1
 *
 * The driver will then automatically configure GPIO40 as the UART0 RX pin.
 */

/* LCD */

#define GPIO_LCD_CLK_0             (GPIO_ALT_M | GPIO_PIN(0))
#define GPIO_LCD_VSYNC_0           (GPIO_ALT_M | GPIO_PIN(1))
#define GPIO_LCD_HSYNC_0           (GPIO_ALT_M | GPIO_PIN(2))
#define GPIO_LCD_EN_0              (GPIO_ALT_M | GPIO_PIN(3))
#define GPIO_LCD_D0_0              (GPIO_ALT_M | GPIO_PIN(4))
#define GPIO_LCD_D1_0              (GPIO_ALT_M | GPIO_PIN(5))
#define GPIO_LCD_D2_0              (GPIO_ALT_M | GPIO_PIN(6))
#define GPIO_LCD_D3_0              (GPIO_ALT_M | GPIO_PIN(7))
#define GPIO_LCD_D4_0              (GPIO_ALT_M | GPIO_PIN(8))
#define GPIO_LCD_D5_0              (GPIO_ALT_M | GPIO_PIN(9))
#define GPIO_LCD_D6_0              (GPIO_ALT_M | GPIO_PIN(10))
#define GPIO_LCD_D7_0              (GPIO_ALT_M | GPIO_PIN(11))
#define GPIO_LCD_D8_0              (GPIO_ALT_M | GPIO_PIN(12))
#define GPIO_LCD_D9_0              (GPIO_ALT_M | GPIO_PIN(13))
#define GPIO_LCD_D10_0             (GPIO_ALT_M | GPIO_PIN(14))
#define GPIO_LCD_D11_0             (GPIO_ALT_M | GPIO_PIN(15))
#define GPIO_LCD_D12_0             (GPIO_ALT_M | GPIO_PIN(16))
#define GPIO_LCD_D13_0             (GPIO_ALT_M | GPIO_PIN(17))
#define GPIO_LCD_D14_0             (GPIO_ALT_M | GPIO_PIN(18))
#define GPIO_LCD_D15_0             (GPIO_ALT_M | GPIO_PIN(19))
#define GPIO_LCD_D16_0             (GPIO_ALT_M | GPIO_PIN(20))
#define GPIO_LCD_D17_0             (GPIO_ALT_M | GPIO_PIN(21))
#define GPIO_LCD_D18_0             (GPIO_ALT_M | GPIO_PIN(22))
#define GPIO_LCD_D19_0             (GPIO_ALT_M | GPIO_PIN(23))
#define GPIO_LCD_D20_0             (GPIO_ALT_M | GPIO_PIN(24))
#define GPIO_LCD_D21_0             (GPIO_ALT_M | GPIO_PIN(25))
#define GPIO_LCD_D22_0             (GPIO_ALT_M | GPIO_PIN(26))
#define GPIO_LCD_D23_0             (GPIO_ALT_M | GPIO_PIN(27))

/* Local I/O */

#define GPIO_LIOA0_0               (GPIO_ALT_2 | GPIO_PIN(1))
#define GPIO_LIOA1_0               (GPIO_ALT_2 | GPIO_PIN(2))
#define GPIO_LIOA2_0               (GPIO_ALT_2 | GPIO_PIN(3))
#define GPIO_LIOA3_0               (GPIO_ALT_2 | GPIO_PIN(4))
#define GPIO_LIOA4_0               (GPIO_ALT_2 | GPIO_PIN(5))
#define GPIO_LIOA5_0               (GPIO_ALT_2 | GPIO_PIN(6))
#define GPIO_LIOA6_0               (GPIO_ALT_2 | GPIO_PIN(7))
#define GPIO_LIOA7_0               (GPIO_ALT_2 | GPIO_PIN(8))
#define GPIO_LIOA8_0               (GPIO_ALT_2 | GPIO_PIN(9))
#define GPIO_LIOA9_0               (GPIO_ALT_2 | GPIO_PIN(10))
#define GPIO_LIOA10_0              (GPIO_ALT_2 | GPIO_PIN(11))
#define GPIO_LIOA11_0              (GPIO_ALT_2 | GPIO_PIN(12))
#define GPIO_LIOA12_0              (GPIO_ALT_2 | GPIO_PIN(13))
#define GPIO_LIOA13_0              (GPIO_ALT_2 | GPIO_PIN(14))
#define GPIO_LIOA14_0              (GPIO_ALT_2 | GPIO_PIN(15))
#define GPIO_LIOA15_0              (GPIO_ALT_2 | GPIO_PIN(16))
#define GPIO_LIOA16_0              (GPIO_ALT_2 | GPIO_PIN(17))
#define GPIO_LIOA17_0              (GPIO_ALT_2 | GPIO_PIN(18))
#define GPIO_LIOA18_0              (GPIO_ALT_2 | GPIO_PIN(19))
#define GPIO_LIOA19_0              (GPIO_ALT_2 | GPIO_PIN(20))
#define GPIO_LIOA20_0              (GPIO_ALT_2 | GPIO_PIN(21))
#define GPIO_LIOA21_0              (GPIO_ALT_2 | GPIO_PIN(22))
#define GPIO_LIOA22_0              (GPIO_ALT_2 | GPIO_PIN(23))

#define GPIO_LIOCSN0_0             (GPIO_ALT_2 | GPIO_PIN(24))
#define GPIO_LIOCSN1_0             (GPIO_ALT_2 | GPIO_PIN(25))
#define GPIO_LIOWRN_0              (GPIO_ALT_2 | GPIO_PIN(26))
#define GPIO_LIORDN_0              (GPIO_ALT_2 | GPIO_PIN(27))

#define GPIO_LIO_DATA0_0           (GPIO_ALT_2 | GPIO_PIN(40))
#define GPIO_LIO_DATA1_0           (GPIO_ALT_2 | GPIO_PIN(41))
#define GPIO_LIO_DATA2_0           (GPIO_ALT_2 | GPIO_PIN(42))
#define GPIO_LIO_DATA3_0           (GPIO_ALT_2 | GPIO_PIN(43))
#define GPIO_LIO_DATA4_0           (GPIO_ALT_2 | GPIO_PIN(44))
#define GPIO_LIO_DATA5_0           (GPIO_ALT_2 | GPIO_PIN(45))
#define GPIO_LIO_DATA6_0           (GPIO_ALT_2 | GPIO_PIN(46))
#define GPIO_LIO_DATA7_0           (GPIO_ALT_2 | GPIO_PIN(47))
#define GPIO_LIO_DATA8_0           (GPIO_ALT_2 | GPIO_PIN(48))
#define GPIO_LIO_DATA9_0           (GPIO_ALT_2 | GPIO_PIN(49))
#define GPIO_LIO_DATA10_0          (GPIO_ALT_2 | GPIO_PIN(50))
#define GPIO_LIO_DATA11_0          (GPIO_ALT_2 | GPIO_PIN(51))
#define GPIO_LIO_DATA12_0          (GPIO_ALT_2 | GPIO_PIN(52))
#define GPIO_LIO_DATA13_0          (GPIO_ALT_2 | GPIO_PIN(53))
#define GPIO_LIO_DATA14_0          (GPIO_ALT_2 | GPIO_PIN(54))
#define GPIO_LIO_DATA15_0          (GPIO_ALT_2 | GPIO_PIN(55))

/* Ethernet MAC */

#define GPIO_GMAC0_RXCTL_0         (GPIO_ALT_M | GPIO_PIN(28))
#define GPIO_GMAC0_RXD0_0          (GPIO_ALT_M | GPIO_PIN(29))
#define GPIO_GMAC0_RXD1_0          (GPIO_ALT_M | GPIO_PIN(30))
#define GPIO_GMAC0_RXD2_0          (GPIO_ALT_M | GPIO_PIN(31))
#define GPIO_GMAC0_RXD3_0          (GPIO_ALT_M | GPIO_PIN(32))
#define GPIO_GMAC0_TXCTL_0         (GPIO_ALT_M | GPIO_PIN(33))
#define GPIO_GMAC0_TXD0_0          (GPIO_ALT_M | GPIO_PIN(34))
#define GPIO_GMAC0_TXD1_0          (GPIO_ALT_M | GPIO_PIN(35))
#define GPIO_GMAC0_TXD2_0          (GPIO_ALT_M | GPIO_PIN(36))
#define GPIO_GMAC0_TXD3_0          (GPIO_ALT_M | GPIO_PIN(37))
#define GPIO_GMAC0_MDCK_0          (GPIO_ALT_M | GPIO_PIN(38))
#define GPIO_GMAC0_MDIO_0          (GPIO_ALT_M | GPIO_PIN(39))
#define GPIO_GMAC0_PTP_TRIG_0      (GPIO_ALT_1 | GPIO_PIN(40))
#define GPIO_GMAC0_PTP_PPS_0       (GPIO_ALT_1 | GPIO_PIN(41))
#define GPIO_GMAC0_COL_0           (GPIO_ALT_2 | GPIO_PIN(72))
#define GPIO_GMAC0_CRS_0           (GPIO_ALT_2 | GPIO_PIN(73))

#define GPIO_GMAC1_RXCTL_0         (GPIO_ALT_1 | GPIO_PIN(44))
#define GPIO_GMAC1_RXD0_0          (GPIO_ALT_1 | GPIO_PIN(45))
#define GPIO_GMAC1_RXD1_0          (GPIO_ALT_1 | GPIO_PIN(46))
#define GPIO_GMAC1_RXD2_0          (GPIO_ALT_1 | GPIO_PIN(47))
#define GPIO_GMAC1_RXD3_0          (GPIO_ALT_1 | GPIO_PIN(48))
#define GPIO_GMAC1_TXCTL_0         (GPIO_ALT_1 | GPIO_PIN(49))
#define GPIO_GMAC1_TXD0_0          (GPIO_ALT_1 | GPIO_PIN(50))
#define GPIO_GMAC1_TXD1_0          (GPIO_ALT_1 | GPIO_PIN(51))
#define GPIO_GMAC1_TXD2_0          (GPIO_ALT_1 | GPIO_PIN(52))
#define GPIO_GMAC1_TXD3_0          (GPIO_ALT_1 | GPIO_PIN(53))
#define GPIO_GMAC1_MDCK_0          (GPIO_ALT_1 | GPIO_PIN(54))
#define GPIO_GMAC1_MDIO_0          (GPIO_ALT_1 | GPIO_PIN(55))
#define GPIO_GMAC1_PTP_TRIG_0      (GPIO_ALT_1 | GPIO_PIN(42))
#define GPIO_GMAC1_PTP_PPS_0       (GPIO_ALT_1 | GPIO_PIN(43))
#define GPIO_GMAC1_COL_0           (GPIO_ALT_2 | GPIO_PIN(74))
#define GPIO_GMAC1_CRS_0           (GPIO_ALT_2 | GPIO_PIN(75))

/* UART */

#define GPIO_UART0_RX_1            (GPIO_ALT_M | GPIO_PIN(40))
#define GPIO_UART0_RX_2            (GPIO_ALT_2 | GPIO_PIN(92))
#define GPIO_UART0_TX_1            (GPIO_ALT_M | GPIO_PIN(41))
#define GPIO_UART0_TX_2            (GPIO_ALT_2 | GPIO_PIN(93))
#define GPIO_UART0_RTS_0           (GPIO_ALT_2 | GPIO_PIN(60))
#define GPIO_UART0_CTS_0           (GPIO_ALT_2 | GPIO_PIN(61))
#define GPIO_UART0_DSR_0           (GPIO_ALT_2 | GPIO_PIN(62))
#define GPIO_UART0_DTR_0           (GPIO_ALT_2 | GPIO_PIN(63))
#define GPIO_UART0_DCD_0           (GPIO_ALT_2 | GPIO_PIN(64))
#define GPIO_UART0_RI_0            (GPIO_ALT_2 | GPIO_PIN(65))

#define GPIO_UART1_RX_1            (GPIO_ALT_M | GPIO_PIN(42))
#define GPIO_UART1_RX_2            (GPIO_ALT_2 | GPIO_PIN(94))
#define GPIO_UART1_TX_1            (GPIO_ALT_M | GPIO_PIN(43))
#define GPIO_UART1_TX_2            (GPIO_ALT_2 | GPIO_PIN(95))
#define GPIO_UART1_RTS_0           (GPIO_ALT_2 | GPIO_PIN(66))
#define GPIO_UART1_CTS_0           (GPIO_ALT_2 | GPIO_PIN(67))
#define GPIO_UART1_DSR_0           (GPIO_ALT_2 | GPIO_PIN(68))
#define GPIO_UART1_DTR_0           (GPIO_ALT_2 | GPIO_PIN(69))
#define GPIO_UART1_DCD_0           (GPIO_ALT_2 | GPIO_PIN(70))
#define GPIO_UART1_RI_0            (GPIO_ALT_2 | GPIO_PIN(71))

#define GPIO_UART2_TX_1            (GPIO_ALT_M | GPIO_PIN(44))
#define GPIO_UART2_TX_2            (GPIO_ALT_2 | GPIO_PIN(96))
#define GPIO_UART2_RX_1            (GPIO_ALT_M | GPIO_PIN(45))
#define GPIO_UART2_RX_2            (GPIO_ALT_2 | GPIO_PIN(97))

#define GPIO_UART3_TX_1            (GPIO_ALT_M | GPIO_PIN(46))
#define GPIO_UART3_TX_2            (GPIO_ALT_2 | GPIO_PIN(98))
#define GPIO_UART3_RX_1            (GPIO_ALT_M | GPIO_PIN(47))
#define GPIO_UART3_RX_2            (GPIO_ALT_2 | GPIO_PIN(99))

#define GPIO_UART4_RX_0            (GPIO_ALT_2 | GPIO_PIN(62))
#define GPIO_UART4_TX_0            (GPIO_ALT_2 | GPIO_PIN(63))
#define GPIO_UART4_CTS_0           (GPIO_ALT_2 | GPIO_PIN(64))
#define GPIO_UART4_RTS_0           (GPIO_ALT_2 | GPIO_PIN(65))

#define GPIO_UART5_RX_0            (GPIO_ALT_2 | GPIO_PIN(64))
#define GPIO_UART5_TX_0            (GPIO_ALT_2 | GPIO_PIN(65))

#define GPIO_UART6_TX_0            (GPIO_ALT_2 | GPIO_PIN(60))
#define GPIO_UART6_RX_0            (GPIO_ALT_2 | GPIO_PIN(61))

#define GPIO_UART7_RX_0            (GPIO_ALT_2 | GPIO_PIN(68))
#define GPIO_UART7_TX_0            (GPIO_ALT_2 | GPIO_PIN(69))
#define GPIO_UART7_CTS_0           (GPIO_ALT_2 | GPIO_PIN(70))
#define GPIO_UART7_RTS_0           (GPIO_ALT_2 | GPIO_PIN(71))

#define GPIO_UART8_RX_0            (GPIO_ALT_2 | GPIO_PIN(70))
#define GPIO_UART8_TX_0            (GPIO_ALT_2 | GPIO_PIN(71))

#define GPIO_UART9_TX_0            (GPIO_ALT_2 | GPIO_PIN(66))
#define GPIO_UART9_RX_0            (GPIO_ALT_2 | GPIO_PIN(67))

/* I2C */

#define GPIO_I2C0_SCL_1            (GPIO_ALT_M | GPIO_PIN(48))
#define GPIO_I2C0_SCL_2            (GPIO_ALT_1 | GPIO_PIN(60))
#define GPIO_I2C0_SDA_1            (GPIO_ALT_M | GPIO_PIN(49))
#define GPIO_I2C0_SDA_2            (GPIO_ALT_1 | GPIO_PIN(61))

#define GPIO_I2C1_SCL_1            (GPIO_ALT_M | GPIO_PIN(50))
#define GPIO_I2C1_SCL_2            (GPIO_ALT_1 | GPIO_PIN(62))
#define GPIO_I2C1_SDA_1            (GPIO_ALT_M | GPIO_PIN(51))
#define GPIO_I2C1_SDA_2            (GPIO_ALT_1 | GPIO_PIN(63))

#define GPIO_I2C2_SCL_1            (GPIO_ALT_M | GPIO_PIN(52))
#define GPIO_I2C2_SCL_2            (GPIO_ALT_2 | GPIO_PIN(82))
#define GPIO_I2C2_SDA_1            (GPIO_ALT_M | GPIO_PIN(53))
#define GPIO_I2C2_SDA_2            (GPIO_ALT_2 | GPIO_PIN(83))

#define GPIO_I2C3_SCL_1            (GPIO_ALT_M | GPIO_PIN(54))
#define GPIO_I2C3_SCL_2            (GPIO_ALT_2 | GPIO_PIN(84))
#define GPIO_I2C3_SDA_1            (GPIO_ALT_M | GPIO_PIN(55))
#define GPIO_I2C3_SDA_2            (GPIO_ALT_2 | GPIO_PIN(85))

/* SPI-FLASH */

#define GPIO_SPI0_CLK_0            (GPIO_ALT_M | GPIO_PIN(56))
#define GPIO_SPI0_MISO_0           (GPIO_ALT_M | GPIO_PIN(57))
#define GPIO_SPI0_MOSI_0           (GPIO_ALT_M | GPIO_PIN(58))
#define GPIO_SPI0_CS0_0            (GPIO_ALT_M | GPIO_PIN(59))
#define GPIO_SPI0_CS1_0            (GPIO_ALT_1 | GPIO_PIN(68))
#define GPIO_SPI0_CS2_0            (GPIO_ALT_1 | GPIO_PIN(69))
#define GPIO_SPI0_CS3_0            (GPIO_ALT_1 | GPIO_PIN(70))

#define GPIO_SPI1_CLK_0            (GPIO_ALT_M | GPIO_PIN(60))
#define GPIO_SPI1_MISO_0           (GPIO_ALT_M | GPIO_PIN(61))
#define GPIO_SPI1_MOSI_0           (GPIO_ALT_M | GPIO_PIN(62))
#define GPIO_SPI1_CS0_0            (GPIO_ALT_M | GPIO_PIN(63))
#define GPIO_SPI1_CS1_0            (GPIO_ALT_2 | GPIO_PIN(78))
#define GPIO_SPI1_CS2_0            (GPIO_ALT_2 | GPIO_PIN(79))
#define GPIO_SPI1_CS3_0            (GPIO_ALT_2 | GPIO_PIN(80))

/* SPI-IO */

#define GPIO_SPIIO0_CLK_0          (GPIO_ALT_M | GPIO_PIN(64))
#define GPIO_SPIIO0_MISO_0         (GPIO_ALT_M | GPIO_PIN(65))
#define GPIO_SPIIO0_MOSI_0         (GPIO_ALT_M | GPIO_PIN(66))
#define GPIO_SPIIO0_CS_0           (GPIO_ALT_M | GPIO_PIN(67))

#define GPIO_SPIIO1_CLK_0          (GPIO_ALT_1 | GPIO_PIN(82))
#define GPIO_SPIIO1_MISO_0         (GPIO_ALT_1 | GPIO_PIN(83))
#define GPIO_SPIIO1_MOSI_0         (GPIO_ALT_1 | GPIO_PIN(84))
#define GPIO_SPIIO1_CS_0           (GPIO_ALT_1 | GPIO_PIN(85))

/* CAN0 */

#define GPIO_CAN0_RX_1             (GPIO_ALT_M | GPIO_PIN(68))
#define GPIO_CAN0_RX_2             (GPIO_ALT_1 | GPIO_PIN(36))
#define GPIO_CAN0_RX_3             (GPIO_ALT_1 | GPIO_PIN(92))
#define GPIO_CAN0_TX_1             (GPIO_ALT_M | GPIO_PIN(69))
#define GPIO_CAN0_TX_2             (GPIO_ALT_1 | GPIO_PIN(37))
#define GPIO_CAN0_TX_3             (GPIO_ALT_1 | GPIO_PIN(93))

/* CAN1 */

#define GPIO_CAN1_RX_1             (GPIO_ALT_M | GPIO_PIN(70))
#define GPIO_CAN1_RX_2             (GPIO_ALT_1 | GPIO_PIN(38))
#define GPIO_CAN1_RX_3             (GPIO_ALT_1 | GPIO_PIN(94))
#define GPIO_CAN1_TX_1             (GPIO_ALT_M | GPIO_PIN(71))
#define GPIO_CAN1_TX_2             (GPIO_ALT_1 | GPIO_PIN(39))
#define GPIO_CAN1_TX_3             (GPIO_ALT_1 | GPIO_PIN(95))

/* CAN2 */

#define GPIO_CAN2_RX_1             (GPIO_ALT_M | GPIO_PIN(72))
#define GPIO_CAN2_RX_2             (GPIO_ALT_2 | GPIO_PIN(56))
#define GPIO_CAN2_RX_3             (GPIO_ALT_1 | GPIO_PIN(96))
#define GPIO_CAN2_TX_1             (GPIO_ALT_M | GPIO_PIN(73))
#define GPIO_CAN2_TX_2             (GPIO_ALT_2 | GPIO_PIN(57))
#define GPIO_CAN2_TX_3             (GPIO_ALT_1 | GPIO_PIN(97))

/* CAN3 */

#define GPIO_CAN3_RX_1             (GPIO_ALT_M | GPIO_PIN(74))
#define GPIO_CAN3_RX_2             (GPIO_ALT_2 | GPIO_PIN(58))
#define GPIO_CAN3_RX_3             (GPIO_ALT_1 | GPIO_PIN(98))
#define GPIO_CAN3_TX_1             (GPIO_ALT_M | GPIO_PIN(75))
#define GPIO_CAN3_TX_2             (GPIO_ALT_2 | GPIO_PIN(59))
#define GPIO_CAN3_TX_3             (GPIO_ALT_1 | GPIO_PIN(99))

/* I2S */

#define GPIO_I2S_MCLK_0            (GPIO_ALT_M | GPIO_PIN(76))
#define GPIO_I2S_BCLK_0            (GPIO_ALT_M | GPIO_PIN(77))
#define GPIO_I2S_LR_0              (GPIO_ALT_M | GPIO_PIN(78))
#define GPIO_I2S_DI_0              (GPIO_ALT_M | GPIO_PIN(79))
#define GPIO_I2S_DO_0              (GPIO_ALT_M | GPIO_PIN(80))

/* PWM0 */

#define GPIO_PWM0_1                (GPIO_ALT_1 | GPIO_PIN(64))
#define GPIO_PWM0_2                (GPIO_ALT_2 | GPIO_PIN(86))
#define GPIO_PWM0_3                (GPIO_ALT_2 | GPIO_PIN(102))

/* PWM1 */

#define GPIO_PWM1_1                (GPIO_ALT_1 | GPIO_PIN(65))
#define GPIO_PWM1_2                (GPIO_ALT_2 | GPIO_PIN(87))
#define GPIO_PWM1_3                (GPIO_ALT_2 | GPIO_PIN(103))

/* PWM2 */

#define GPIO_PWM2_1                (GPIO_ALT_1 | GPIO_PIN(66))
#define GPIO_PWM2_2                (GPIO_ALT_2 | GPIO_PIN(88))
#define GPIO_PWM2_3                (GPIO_ALT_2 | GPIO_PIN(104))

/* PWM3 */

#define GPIO_PWM3_1                (GPIO_ALT_1 | GPIO_PIN(67))
#define GPIO_PWM3_2                (GPIO_ALT_2 | GPIO_PIN(89))
#define GPIO_PWM3_3                (GPIO_ALT_2 | GPIO_PIN(105))

/* ATIM (Advanced Timer) */

#define GPIO_TIM1_CH1_1            (GPIO_ALT_M | GPIO_PIN(81))
#define GPIO_TIM1_CH1_2            (GPIO_ALT_2 | GPIO_PIN(28))
#define GPIO_TIM1_CH2_1            (GPIO_ALT_M | GPIO_PIN(82))
#define GPIO_TIM1_CH2_2            (GPIO_ALT_2 | GPIO_PIN(29))
#define GPIO_TIM1_CH3_1            (GPIO_ALT_M | GPIO_PIN(83))
#define GPIO_TIM1_CH3_2            (GPIO_ALT_2 | GPIO_PIN(30))
#define GPIO_TIM1_CH1N_1           (GPIO_ALT_M | GPIO_PIN(84))
#define GPIO_TIM1_CH1N_2           (GPIO_ALT_2 | GPIO_PIN(31))
#define GPIO_TIM1_CH2N_1           (GPIO_ALT_M | GPIO_PIN(85))
#define GPIO_TIM1_CH2N_2           (GPIO_ALT_2 | GPIO_PIN(32))
#define GPIO_TIM1_CH3N_1           (GPIO_ALT_M | GPIO_PIN(86))
#define GPIO_TIM1_CH3N_2           (GPIO_ALT_2 | GPIO_PIN(33))
#define GPIO_TIM1_CH4_1            (GPIO_ALT_1 | GPIO_PIN(76))
#define GPIO_TIM1_CH4_2            (GPIO_ALT_1 | GPIO_PIN(101))
#define GPIO_TIM1_ETR_1            (GPIO_ALT_1 | GPIO_PIN(78))
#define GPIO_TIM1_ETR_2            (GPIO_ALT_1 | GPIO_PIN(103))
#define GPIO_TIM1_BKIN_1           (GPIO_ALT_1 | GPIO_PIN(80))
#define GPIO_TIM1_BKIN_2           (GPIO_ALT_1 | GPIO_PIN(105))

/* GTIM (General Timer) */

#define GPIO_TIM2_CH1_1            (GPIO_ALT_M | GPIO_PIN(87))
#define GPIO_TIM2_CH1_2            (GPIO_ALT_2 | GPIO_PIN(34))
#define GPIO_TIM2_CH2_1            (GPIO_ALT_M | GPIO_PIN(88))
#define GPIO_TIM2_CH2_2            (GPIO_ALT_2 | GPIO_PIN(35))
#define GPIO_TIM2_CH3_1            (GPIO_ALT_M | GPIO_PIN(89))
#define GPIO_TIM2_CH3_2            (GPIO_ALT_2 | GPIO_PIN(36))
#define GPIO_TIM2_CH4_1            (GPIO_ALT_1 | GPIO_PIN(77))
#define GPIO_TIM2_CH4_2            (GPIO_ALT_1 | GPIO_PIN(102))
#define GPIO_TIM2_ETR_1            (GPIO_ALT_1 | GPIO_PIN(79))
#define GPIO_TIM2_ETR_2            (GPIO_ALT_1 | GPIO_PIN(104))

/* SDIO0 */

#define GPIO_SDIO0_CLK_0           (GPIO_ALT_M | GPIO_PIN(90))
#define GPIO_SDIO0_CMD_0           (GPIO_ALT_M | GPIO_PIN(91))
#define GPIO_SDIO0_D0_0            (GPIO_ALT_M | GPIO_PIN(92))
#define GPIO_SDIO0_D1_0            (GPIO_ALT_M | GPIO_PIN(93))
#define GPIO_SDIO0_D2_0            (GPIO_ALT_M | GPIO_PIN(94))
#define GPIO_SDIO0_D3_0            (GPIO_ALT_M | GPIO_PIN(95))
#define GPIO_SDIO0_D4_0            (GPIO_ALT_M | GPIO_PIN(96))
#define GPIO_SDIO0_D5_0            (GPIO_ALT_M | GPIO_PIN(97))
#define GPIO_SDIO0_D6_0            (GPIO_ALT_M | GPIO_PIN(98))
#define GPIO_SDIO0_D7_0            (GPIO_ALT_M | GPIO_PIN(99))

/* SDIO1 */

#define GPIO_SDIO1_CLK_0           (GPIO_ALT_M | GPIO_PIN(100))
#define GPIO_SDIO1_CMD_0           (GPIO_ALT_M | GPIO_PIN(101))
#define GPIO_SDIO1_D0_0            (GPIO_ALT_M | GPIO_PIN(102))
#define GPIO_SDIO1_D1_0            (GPIO_ALT_M | GPIO_PIN(103))
#define GPIO_SDIO1_D2_0            (GPIO_ALT_M | GPIO_PIN(104))
#define GPIO_SDIO1_D3_0            (GPIO_ALT_M | GPIO_PIN(105))
#define GPIO_SDIO1_D4_1            (GPIO_ALT_1 | GPIO_PIN(72))
#define GPIO_SDIO1_D4_2            (GPIO_ALT_1 | GPIO_PIN(86))
#define GPIO_SDIO1_D5_1            (GPIO_ALT_1 | GPIO_PIN(73))
#define GPIO_SDIO1_D5_2            (GPIO_ALT_1 | GPIO_PIN(87))
#define GPIO_SDIO1_D6_1            (GPIO_ALT_1 | GPIO_PIN(74))
#define GPIO_SDIO1_D6_2            (GPIO_ALT_1 | GPIO_PIN(88))
#define GPIO_SDIO1_D7_1            (GPIO_ALT_1 | GPIO_PIN(75))
#define GPIO_SDIO1_D7_2            (GPIO_ALT_1 | GPIO_PIN(89))

#endif /* __ARCH_LOONGARCH_SRC_LS2K0300_HARDWARE_LS2K0300_PINMAP_H */
