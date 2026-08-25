/****************************************************************************
 * arch/loongarch/src/ls2k/ls_config.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_CONFIG_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_CONFIG_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <arch/board/board.h>

#include "hardware/ls_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#undef HAVE_UART_DEVICE
#if CONFIG_16550_UART
#  define HAVE_UART_DEVICE 1
#endif

#if defined(CONFIG_16550_UART0_SERIAL_CONSOLE)
#  define HAVE_SERIAL_CONSOLE 1
#  define LS_CONSOLE_BASE       LS_UART0_BASE
#  define LS_CONSOLE_BAUD       CONFIG_16550_UART0_BAUD
#  define LS_CONSOLE_BITS       CONFIG_16550_UART0_BITS
#  define LS_CONSOLE_PARITY     CONFIG_16550_UART0_PARITY
#  define LS_CONSOLE_2STOP      CONFIG_16550_UART0_2STOP
#  define HAVE_UART
#elif defined(CONFIG_16550_UART1_SERIAL_CONSOLE)
#  define HAVE_SERIAL_CONSOLE 1
#  define LS_CONSOLE_BASE       LS_UART1_BASE
#  define LS_CONSOLE_BAUD       CONFIG_16550_UART1_BAUD
#  define LS_CONSOLE_BITS       CONFIG_16550_UART1_BITS
#  define LS_CONSOLE_PARITY     CONFIG_16550_UART1_PARITY
#  define LS_CONSOLE_2STOP      CONFIG_16550_UART1_2STOP
#  define HAVE_UART
#else
#  undef HAVE_SERIAL_CONSOLE
#endif

#ifndef LS_CONSOLE_BASE
#  define LS_CONSOLE_BASE       LS_UART0_BASE
#endif

#ifndef LS_CONSOLE_BAUD
#  define LS_CONSOLE_BAUD       115200
#endif

#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_CONFIG_H */
