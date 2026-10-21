/****************************************************************************
 * boards/loongarch/ls2k/hummingbird-ls2k0300/include/board.h
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

#ifndef __BOARDS_LOONGARCH_LS2K_HUMMINGBIRD_LS2K0300_INCLUDE_BOARD_H
#define __BOARDS_LOONGARCH_LS2K_HUMMINGBIRD_LS2K0300_INCLUDE_BOARD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <stdint.h>
#  include <stdbool.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Clocking *****************************************************************/

/* On-board crystal frequency is 120MHz */

#define LS_BOARD_XTAL             120000000ul

#define LS_NODE_PLLSRC            LS_BOARD_XTAL
#define LS_NODE_PLLLOOPC          (100)
#define LS_NODE_PLLREFC           (6)
#define LS_NODE_VCO               (LS_NODE_PLLSRC * LS_NODE_PLLLOOPC / LS_NODE_PLLREFC)
#define LS_NODE_PLLDIV            (2)
#define LS_NODE_FREQUENCY         (LS_NODE_VCO / LS_NODE_PLLDIV)
#define LS_GMAC_PLLDIV            (16)
#define LS_GMAC_FREQUENCY         (LS_NODE_VCO / LS_GMAC_PLLDIV)
#define LS_I2S_PLLDIV             (20)
#define LS_I2S_FREQUENCY          (LS_NODE_VCO / LS_I2S_PLLDIV)

#define LS_DDR_PLLSRC             LS_BOARD_XTAL
#define LS_DDR_PLLLOOPC           (40)
#define LS_DDR_PLLREFC            (3)
#define LS_DDR_VCO                (LS_DDR_PLLSRC * LS_DDR_PLLLOOPC / LS_DDR_PLLREFC)
#define LS_DDR_PLLDIV             (2)
#define LS_DDR_FREQUENCY          (LS_DDR_VCO / LS_DDR_PLLDIV)
#define LS_NET_PLLDIV             (8)
#define LS_NET_FREQUENCY          (LS_DDR_VCO / LS_NET_PLLDIV)
#define LS_DEV_PLLDIV             (8)
#define LS_DEV_FREQUENCY          (LS_DDR_VCO / LS_DEV_PLLDIV)

#define LS_PIX_PLLSRC             LS_BOARD_XTAL
#define LS_PIX_PLLLOOPC           (80)
#define LS_PIX_PLLREFC            (4)
#define LS_PIX_VCO                (LS_PIX_PLLSRC * LS_PIX_PLLLOOPC / LS_PIX_PLLREFC)
#define LS_PIX_PLLDIV             (20)
#define LS_PIX_FREQUENCY          (LS_PIX_VCO / LS_PIX_PLLDIV)
#define LS_GMACBP_PLLDIV          (16)
#define LS_GMACBP_FREQUENCY       (LS_PIX_VCO / LS_GMACBP_PLLDIV)

#define LS_STABLE_CLK             LS_BOARD_XTAL
#define LS_APB_FREQUENCY          LS_DEV_FREQUENCY

/* LED definitions **********************************************************/

/* The board has 2 LEDs that we will encode as: */
#define LED_STARTED       0  /* No LEDs */
#define LED_HEAPALLOCATE  1  /* LED1 on */
#define LED_IRQSENABLED   2  /* LED2 on */
#define LED_STACKCREATED  3  /* LED1 on */
#define LED_INIRQ         4  /* LED1 off */
#define LED_SIGNAL        5  /* LED2 on */
#define LED_ASSERTION     6  /* LED1 + LED2 */
#define LED_PANIC         7  /* LED1 / LED2 blinking */

/* Alternate function pin selections ****************************************/

/* UART */

#define GPIO_UART0_RX             GPIO_UART0_RX_1
#define GPIO_UART0_TX             GPIO_UART0_TX_1
#define GPIO_UART1_RX             GPIO_UART1_RX_1
#define GPIO_UART1_TX             GPIO_UART1_TX_1
#define GPIO_UART2_TX             GPIO_UART2_TX_1
#define GPIO_UART2_RX             GPIO_UART2_RX_1
#define GPIO_UART3_TX             GPIO_UART3_TX_1
#define GPIO_UART3_RX             GPIO_UART3_RX_1

/* I2C */

#define GPIO_I2C0_SCL             GPIO_I2C0_SCL_1
#define GPIO_I2C0_SDA             GPIO_I2C0_SDA_1
#define GPIO_I2C1_SCL             GPIO_I2C1_SCL_1
#define GPIO_I2C1_SDA             GPIO_I2C1_SDA_1
#define GPIO_I2C2_SCL             GPIO_I2C2_SCL_1
#define GPIO_I2C2_SDA             GPIO_I2C2_SDA_1
#define GPIO_I2C3_SCL             GPIO_I2C3_SCL_1
#define GPIO_I2C3_SDA             GPIO_I2C3_SDA_1

/* SPI */

#define GPIO_SPI0_CLK             GPIO_SPI0_CLK_0
#define GPIO_SPI0_MISO            GPIO_SPI0_MISO_0
#define GPIO_SPI0_MOSI            GPIO_SPI0_MOSI_0
#define GPIO_SPI0_CS              GPIO_SPI0_CS0_0
#define GPIO_SPI1_CLK             GPIO_SPI1_CLK_0
#define GPIO_SPI1_MISO            GPIO_SPI1_MISO_0
#define GPIO_SPI1_MOSI            GPIO_SPI1_MOSI_0
#define GPIO_SPI1_CS              GPIO_SPI1_CS0_0
#define GPIO_SPIIO0_CLK           GPIO_SPIIO0_CLK_0
#define GPIO_SPIIO0_MISO          GPIO_SPIIO0_MISO_0
#define GPIO_SPIIO0_MOSI          GPIO_SPIIO0_MOSI_0
#define GPIO_SPIIO0_CS            GPIO_SPIIO0_CS_0
#define GPIO_SPIIO1_CLK           GPIO_SPIIO1_CLK_0
#define GPIO_SPIIO1_MISO          GPIO_SPIIO1_MISO_0
#define GPIO_SPIIO1_MOSI          GPIO_SPIIO1_MOSI_0
#define GPIO_SPIIO1_CS            GPIO_SPIIO1_CS_0

/* CAN */

#define GPIO_CAN0_RX              GPIO_CAN0_RX_1
#define GPIO_CAN0_TX              GPIO_CAN0_TX_1
#define GPIO_CAN1_RX              GPIO_CAN1_RX_1
#define GPIO_CAN1_TX              GPIO_CAN1_TX_1
#define GPIO_CAN2_RX              GPIO_CAN2_RX_1
#define GPIO_CAN2_TX              GPIO_CAN2_TX_1
#define GPIO_CAN3_RX              GPIO_CAN3_RX_1
#define GPIO_CAN3_TX              GPIO_CAN3_TX_1

/* PWM */

#define GPIO_PWM0                 GPIO_PWM0_2
#define GPIO_PWM1                 GPIO_PWM1_2
#define GPIO_PWM2                 GPIO_PWM2_2
#define GPIO_PWM3                 GPIO_PWM3_2

/* TIM */

#define GPIO_TIM1_CH1OUT          GPIO_TIM1_CH1_1
#define GPIO_TIM1_CH2OUT          GPIO_TIM1_CH2_1
#define GPIO_TIM1_CH3OUT          GPIO_TIM1_CH3_1
#define GPIO_TIM1_CH4OUT          GPIO_TIM1_CH4_1
#define GPIO_TIM2_CH1OUT          GPIO_TIM2_CH1_1
#define GPIO_TIM2_CH2OUT          GPIO_TIM2_CH2_1
#define GPIO_TIM2_CH3OUT          GPIO_TIM2_CH3_1
#define GPIO_TIM2_CH4OUT          GPIO_TIM2_CH4_1

#define GPIO_TIM1_CH1IN           GPIO_TIM1_CH1_1
#define GPIO_TIM1_CH2IN           GPIO_TIM1_CH2_1
#define GPIO_TIM1_CH3IN           GPIO_TIM1_CH3_1
#define GPIO_TIM1_CH4IN           GPIO_TIM1_CH4_1
#define GPIO_TIM2_CH1IN           GPIO_TIM2_CH1_1
#define GPIO_TIM2_CH2IN           GPIO_TIM2_CH2_1
#define GPIO_TIM2_CH3IN           GPIO_TIM2_CH3_1
#define GPIO_TIM2_CH4IN           GPIO_TIM2_CH4_1

#define GPIO_TIM1_EXT_CLK_IN      GPIO_TIM1_ETR_1
#define GPIO_TIM2_EXT_CLK_IN      GPIO_TIM2_ETR_1
#define GPIO_TIM1_BREAK_IN        GPIO_TIM1_BKIN_1

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: ls_boardinitialize
 *
 * Description:
 *   All LS architectures must provide the following entry point.  This
 *   entry point is called early in the initialization -- after all memory
 *   has been configured and mapped but before any devices have been
 *   initialized.
 *
 ****************************************************************************/

void ls_boardinitialize(void);

#endif /* __BOARDS_LOONGARCH_LS2K_HUMMINGBIRD_LS2K0300_INCLUDE_BOARD_H */
