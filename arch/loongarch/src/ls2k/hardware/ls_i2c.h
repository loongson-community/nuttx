/****************************************************************************
 * arch/loongarch/src/ls2k/hardware/ls_i2c.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_I2C_H
#define __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_I2C_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets *********************************************************/

#define LS_I2C_CR1_OFFSET       0x0000  /* Control register 1 (32-bit) */
#define LS_I2C_CR2_OFFSET       0x0004  /* Control register 2 (32-bit) */
#define LS_I2C_OAR1_OFFSET      0x0008  /* Own address register 1 (32-bit) */
#define LS_I2C_DR_OFFSET        0x0010  /* Data register (32-bit) */
#define LS_I2C_SR1_OFFSET       0x0014  /* Status register 1 (32-bit) */
#define LS_I2C_SR2_OFFSET       0x0018  /* Status register 2 (32-bit) */
#define LS_I2C_CCR_OFFSET       0x001c  /* Clock control register (32-bit) */
#define LS_I2C_TRISE_OFFSET     0x0020  /* TRISE Register (32-bit) */

/* Register Addresses *******************************************************/

#if LS_NI2C > 0
#  define LS_I2C0_CR1           (LS_I2C0_BASE+LS_I2C_CR1_OFFSET)
#  define LS_I2C0_CR2           (LS_I2C0_BASE+LS_I2C_CR2_OFFSET)
#  define LS_I2C0_OAR1          (LS_I2C0_BASE+LS_I2C_OAR1_OFFSET)
#  define LS_I2C0_DR            (LS_I2C0_BASE+LS_I2C_DR_OFFSET)
#  define LS_I2C0_SR1           (LS_I2C0_BASE+LS_I2C_SR1_OFFSET)
#  define LS_I2C0_SR2           (LS_I2C0_BASE+LS_I2C_SR2_OFFSET)
#  define LS_I2C0_CCR           (LS_I2C0_BASE+LS_I2C_CCR_OFFSET)
#  define LS_I2C0_TRISE         (LS_I2C0_BASE+LS_I2C_TRISE_OFFSET)
#endif

#if LS_NI2C > 1
#  define LS_I2C1_CR1           (LS_I2C1_BASE+LS_I2C_CR1_OFFSET)
#  define LS_I2C1_CR2           (LS_I2C1_BASE+LS_I2C_CR2_OFFSET)
#  define LS_I2C1_OAR1          (LS_I2C1_BASE+LS_I2C_OAR1_OFFSET)
#  define LS_I2C1_DR            (LS_I2C1_BASE+LS_I2C_DR_OFFSET)
#  define LS_I2C1_SR1           (LS_I2C1_BASE+LS_I2C_SR1_OFFSET)
#  define LS_I2C1_SR2           (LS_I2C1_BASE+LS_I2C_SR2_OFFSET)
#  define LS_I2C1_CCR           (LS_I2C1_BASE+LS_I2C_CCR_OFFSET)
#  define LS_I2C1_TRISE         (LS_I2C1_BASE+LS_I2C_TRISE_OFFSET)
#endif

#if LS_NI2C > 2
#  define LS_I2C2_CR1           (LS_I2C2_BASE+LS_I2C_CR1_OFFSET)
#  define LS_I2C2_CR2           (LS_I2C2_BASE+LS_I2C_CR2_OFFSET)
#  define LS_I2C2_OAR1          (LS_I2C2_BASE+LS_I2C_OAR1_OFFSET)
#  define LS_I2C2_DR            (LS_I2C2_BASE+LS_I2C_DR_OFFSET)
#  define LS_I2C2_SR1           (LS_I2C2_BASE+LS_I2C_SR1_OFFSET)
#  define LS_I2C2_SR2           (LS_I2C2_BASE+LS_I2C_SR2_OFFSET)
#  define LS_I2C2_CCR           (LS_I2C2_BASE+LS_I2C_CCR_OFFSET)
#  define LS_I2C2_TRISE         (LS_I2C2_BASE+LS_I2C_TRISE_OFFSET)
#endif

#if LS_NI2C > 3
#  define LS_I2C3_CR1           (LS_I2C3_BASE+LS_I2C_CR1_OFFSET)
#  define LS_I2C3_CR2           (LS_I2C3_BASE+LS_I2C_CR2_OFFSET)
#  define LS_I2C3_OAR1          (LS_I2C3_BASE+LS_I2C_OAR1_OFFSET)
#  define LS_I2C3_DR            (LS_I2C3_BASE+LS_I2C_DR_OFFSET)
#  define LS_I2C3_SR1           (LS_I2C3_BASE+LS_I2C_SR1_OFFSET)
#  define LS_I2C3_SR2           (LS_I2C3_BASE+LS_I2C_SR2_OFFSET)
#  define LS_I2C3_CCR           (LS_I2C3_BASE+LS_I2C_CCR_OFFSET)
#  define LS_I2C3_TRISE         (LS_I2C3_BASE+LS_I2C_TRISE_OFFSET)
#endif

/* Register Bitfield Definitions ********************************************/

/* Control register 1 */

#define I2C_CR1_PE              (1 << 0)  /* Bit 0: Peripheral Enable */
                                          /* Bits 1-5: Reserved */
#define I2C_CR1_ENGC            (1 << 6)  /* Bit 6: General Call Enable */
#define I2C_CR1_NOSTRETCH       (1 << 7)  /* Bit 7: Clock Stretching Disable (Slave mode) */
#define I2C_CR1_START           (1 << 8)  /* Bit 8: Start Generation */
#define I2C_CR1_STOP            (1 << 9)  /* Bit 9: Stop Generation */
#define I2C_CR1_ACK             (1 << 10) /* Bit 10: Acknowledge Enable */
#define I2C_CR1_POS             (1 << 11) /* Bit 11: Acknowledge/PEC Position (for data reception) */
                                          /* Bits 12-13: Reserved */
#define I2C_CR1_RECOVER         (1 << 14) /* Bit 14: Bus Recover */
#define I2C_CR1_SWRST           (1 << 15) /* Bit 15: Software Reset */

/* Control register 2 */

#define I2C_CR2_FREQ_SHIFT      (0)       /* Bits 5-0: Peripheral Clock Frequency */
#define I2C_CR2_FREQ_MASK       (0x3f << I2C_CR2_FREQ_SHIFT)
                                          /* Bits 6-7: Reserved */
#define I2C_CR2_ITERREN         (1 << 8)  /* Bit 8: Error Interrupt Enable */
#define I2C_CR2_ITEVFEN         (1 << 9)  /* Bit 9: Event Interrupt Enable */
#define I2C_CR2_ITBUFEN         (1 << 10) /* Bit 10: Buffer Interrupt Enable */
#define I2C_CR2_DMAEN           (1 << 11) /* Bit 11: DMA Requests Enable */
                                          /* Bits 12-31: Reserved */

#define I2C_CR2_ALLINTS         (I2C_CR2_ITERREN|I2C_CR2_ITEVFEN|I2C_CR2_ITBUFEN)

/* Own address register 1 */

                                          /* Bit 0: Reserved */
#define I2C_OAR1_ADD8_SHIFT     (1)       /* Bits 7-1: Interface Address */
#define I2C_OAR1_ADD8_MASK      (0x007f << I2C_OAR1_ADD8_SHIFT)
                                          /* Bits 8-31: Reserved */

/* Data register */

#define I2C_DR_SHIFT            (0)       /* Bits 7-0: 8-bit Data Register */
#define I2C_DR_MASK             (0x00ff << I2C_DR_SHIFT)

/* Status register 1 */

#define I2C_SR1_SB              (1 << 0)  /* Bit 0: Start Bit (Master mode) */
#define I2C_SR1_ADDR            (1 << 1)  /* Bit 1: Address sent (master mode)/matched (slave mode) */
#define I2C_SR1_BTF             (1 << 2)  /* Bit 2: Byte Transfer Finished */

                                          /* Bit 3: Reserved */
#define I2C_SR1_STOPF           (1 << 4)  /* Bit 4: Stop detection (Slave mode) */
                                          /* Bit 5: Reserved */
#define I2C_SR1_RXNE            (1 << 6)  /* Bit 6: Data Register not Empty (receivers) */
#define I2C_SR1_TXE             (1 << 7)  /* Bit 7: Data Register Empty (transmitters) */
#define I2C_SR1_BERR            (1 << 8)  /* Bit 8: Bus Error */
#define I2C_SR1_ARLO            (1 << 9)  /* Bit 9: Arbitration Lost (master mode) */
#define I2C_SR1_AF              (1 << 10) /* Bit 10: Acknowledge Failure */
#define I2C_SR1_OVR             (1 << 11) /* Bit 11: Overrun/Underrun */
                                          /* Bits 12-31: Reserved */

#define I2C_SR1_ERRORMASK       (I2C_SR1_BERR|I2C_SR1_ARLO|I2C_SR1_AF|I2C_SR1_OVR)

/* Status register 2 */

#define I2C_SR2_MSL             (1 << 0)  /* Bit 0: Master/Slave */
#define I2C_SR2_BUSY            (1 << 1)  /* Bit 1: Bus Busy */
#define I2C_SR2_TRA             (1 << 2)  /* Bit 2: Transmitter/Receiver */
                                          /* Bit 3: Reserved */
#define I2C_SR2_GENCALL         (1 << 4)  /* Bit 4: General Call Address (Slave mode) */
                                          /* Bits 5-31: Reserved */

/* Clock control register */

#define I2C_CCR_CCR_SHIFT       (0)       /* Bits 11-0: Clock Control Register in Fast/Standard mode (Master mode) */
#define I2C_CCR_CCR_MASK        (0x0fff << I2C_CCR_CCR_SHIFT)
#define I2C_CCR_DUTY            (1 << 14) /* Bit 14: Fast Mode Duty Cycle */
#define I2C_CCR_FS              (1 << 15) /* Bit 15: Fast Mode Selection */

/* TRISE Register */

#define I2C_TRISE_SHIFT         (0) /* Bits 5-0: Maximum Rise Time in Fast/Standard mode (Master mode) */
#define I2C_TRISE_MASK          (0x3f << I2C_TRISE_SHIFT)

#endif /* __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_I2C_H */
