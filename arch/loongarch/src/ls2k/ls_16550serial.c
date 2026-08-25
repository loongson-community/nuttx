/****************************************************************************
 * arch/loongarch/src/ls2k/ls_16550serial.c
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

#ifdef CONFIG_16550_UART

#include <nuttx/arch.h>
#include <nuttx/serial/uart_16550.h>

#include "loongarch_internal.h"
#include "chip.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: uart_getreg
 *
 * Description:
 *   Map the physical UART base address to uncached space before accessing
 *   the register.  LS2K0300 requires all peripheral accesses to go through
 *   uncached addressing after MMU initialization.
 *
 ****************************************************************************/

uart_datawidth_t uart_getreg(struct u16550_s *priv, unsigned int offset)
{
  uintptr_t addr = PHYS_TO_UNCACHED(priv->uartbase) + offset;
  return *(volatile uart_datawidth_t *)addr;
}

/****************************************************************************
 * Name: uart_putreg
 ****************************************************************************/

void uart_putreg(struct u16550_s *priv, unsigned int offset,
                 uart_datawidth_t value)
{
  uintptr_t addr = PHYS_TO_UNCACHED(priv->uartbase) + offset;
  *(volatile uart_datawidth_t *)addr = value;
}

/****************************************************************************
 * Name: loongarch_earlyserialinit
 ****************************************************************************/

void loongarch_earlyserialinit(void)
{
  u16550_earlyserialinit();
}

/****************************************************************************
 * Name: loongarch_serialinit
 ****************************************************************************/

void loongarch_serialinit(void)
{
  u16550_serialinit();
}

#endif /* CONFIG_16550_UART */
