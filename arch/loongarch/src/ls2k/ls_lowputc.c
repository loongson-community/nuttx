/****************************************************************************
 * arch/loongarch/src/ls2k/ls_lowputc.c
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

#include <nuttx/serial/uart_16550.h>

#include <arch/board/board.h>

#include "loongarch_internal.h"

#include "ls_config.h"
#include "ls_lowputc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* LS_CONSOLE_BASE and LS_CONSOLE_BAUD are set in ls_config.h */

#ifndef LS_CONSOLE_BASE
#  define LS_CONSOLE_BASE      LS_UART0_BASE
#  define LS_CONSOLE_BAUD      115200
#  define HAVE_UART
#endif

#define UART_REF_CLK                 (LS_APB_FREQUENCY)
#define UART_DIV_VAL                 ((UART_REF_CLK + (LS_CONSOLE_BAUD * 8)) / (LS_CONSOLE_BAUD * 16))
#define UART_DIV_HI                  ((UART_DIV_VAL >> 8) & 0xff)
#define UART_DIV_LO                  (UART_DIV_VAL & 0xff)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void loongarch_lowputc(char ch)
{
#ifdef HAVE_UART
  while ((getreg8(LS_CONSOLE_BASE + UART_LSR_OFFSET) & UART_LSR_THRE) == 0)
    {
    }

  putreg8(ch, LS_CONSOLE_BASE + UART_THR_OFFSET);
#endif
}

/****************************************************************************
 * Name: ls_lowsetup
 *
 * Description:
 *   This performs basic initialization of the USART used for the serial
 *   console.  Its purpose is to get the console output available as soon
 *   as possible.
 *
 ****************************************************************************/

void ls_lowsetup(void)
{
#ifdef HAVE_UART
  putreg8(UART_LCR_DLAB, LS_CONSOLE_BASE + UART_LCR_OFFSET);

  putreg8(UART_DIV_LO, LS_CONSOLE_BASE + UART_DLL_OFFSET);
  putreg8(UART_DIV_HI, LS_CONSOLE_BASE + UART_DLM_OFFSET);

  putreg8(UART_LCR_WLS_8BIT, LS_CONSOLE_BASE + UART_LCR_OFFSET);

  putreg8(UART_FCR_RXTRIGGER_4 | UART_FCR_TXRST |
          UART_FCR_RXRST | UART_FCR_FIFOEN,
          LS_CONSOLE_BASE + UART_FCR_OFFSET);
#endif /* HAVE_UART */
}
