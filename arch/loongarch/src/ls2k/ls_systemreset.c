/****************************************************************************
 * arch/loongarch/src/ls2k/ls_systemreset.c
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

#include <stdint.h>
#include <stdbool.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include "loongarch_internal.h"
#include "hardware/ls_memorymap.h"
#include "hardware/ls_wdt.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_systemreset
 *
 * Description:
 *   Perform a system soft reset.
 *
 *   The 2K0300 watchdog (WDT) provides a WD_EN register (offset 0x00).  Its
 *   bit 0 (OS_RST) is a system soft reset bit: writing '1' to it resets the
 *   SoC.  The bit is high-level active and does not depend on the watchdog
 *   enable (bit 1).
 *
 ****************************************************************************/

void up_systemreset(void)
{
  uint32_t reg;

  /* Disable interrupts so that the reset sequence cannot be interrupted */

  (void)up_irq_save();

  /* Read-modify-write to preserve the watchdog enable bit and set the
   * system soft reset bit.
   */

  reg  = getreg32(LS_WDT_BASE + LS_WDT_RST_CTR);
  reg |= WDT_RST_CTR_OS_RST;
  putreg32(reg, LS_WDT_BASE + LS_WDT_RST_CTR);

  /* The system reset should never return.  Wait here. */

  for (; ; );
}
