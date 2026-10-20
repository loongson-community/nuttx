/****************************************************************************
 * arch/loongarch/src/ls2k/ls_i2c_hw.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_I2C_HW_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_I2C_HW_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Polled I2C transfer layer for the Loongson LS2K0300 I2C peripheral.
 * The hw layer touches the hardware only through this port structure;
 * ls_i2c_hw_bind() binds it to direct MMIO (getreg16/putreg16/up_udelay).
 */

struct ls_i2c_hw_port_s
{
  uintptr_t base;                                /* register base address */
  void *ctx;
  uint16_t (*read16)(void *ctx, uintptr_t addr);
  void (*write16)(void *ctx, uintptr_t addr, uint16_t v);
  void (*udelay)(void *ctx, unsigned us);
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Bind a port to direct MMIO at the given register base address. */

void ls_i2c_hw_bind(struct ls_i2c_hw_port_s *port, uintptr_t base);

/* Initialize the peripheral clock/timing (100 kHz standard mode).
 * pclk is hardcoded to 10 MHz; see ls300_i2c_reginit() in the vendor
 * Linux driver.
 */

int ls_i2c_hw_init(struct ls_i2c_hw_port_s *port);

/* Single-message transfer: START -> write or read LEN bytes -> STOP.
 * No repeated start, no 10-bit addressing.
 *
 * Returns 0 on success, -ENXIO on slave NAK, -ETIMEDOUT on flag
 * timeout (after SWRST recovery), -EBUSY when the bus stays busy
 * after one recovery attempt.
 */

int ls_i2c_hw_xfer(struct ls_i2c_hw_port_s *port, uint8_t addr,
                   int read, uint8_t *buf, uint16_t len);

#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_I2C_HW_H */
