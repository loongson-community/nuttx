/****************************************************************************
 * arch/loongarch/src/common/loongarch_exception_common.S
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

#include <nuttx/irq.h>
#include <arch/arch.h>
#include <arch/irq.h>

#include "chip.h"

#include "loongarch_macros.S"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Symbols
 ****************************************************************************/

/****************************************************************************
 * Name: exception_common
 *
 * Description:
 *   Common exception handler. Save context, determine cause, dispatch.
 *
 ****************************************************************************/

  .section .text
  .global  exception_common
  .global return_from_exception
  .type    exception_common, function
  .type    return_from_exception, function
  .align   8

exception_common:
  .cfi_sections .debug_frame
  .cfi_startproc

  addi.d    $sp, $sp, -XCPTCONTEXT_SIZE
  save_ctx  $sp

  addi.d    $s0, $sp, XCPTCONTEXT_SIZE
  REGSTORE  $s0, $sp, REG_SP

  csrrd     $s0, LOONGARCH_CSR_ERA
  REGSTORE  $s0, $sp, REG_EPC

  csrrd     $s0, LOONGARCH_CSR_PRMD
  REGSTORE  $s0, $sp, REG_INT_CTX

  csrrd     $s1, LOONGARCH_CSR_ESTAT       /* s1 = ESTAT */
  srli.d    $s2, $s1, CSR_ESTAT_EXC_SHIFT  /* s2 = Ecode and Esubcode */
  andi      $s2, $s2, 0x3f                 /* s2 = ESTAT.Ecode */

  /* Check whether it is an exception or interrupt */

  bnez      $s2, handle_irq                /* ESTAT.Ecode != 0: this is an exception */

  li.d      $s3, 0x7fff
  and       $s3, $s1, $s3                  /* s3 = ESTAT.IS: set bits = pending interrupts */

  beqz      $s3, .Lspurious_interrupt      /* ESTAT.IS == 0: no pending IRQ, it is spurious */

  ctz.d     $s2, $s3                       /* s2 = index of the lowest pending IRQ bit */
  addi.d    $s2, $s2, EXCCODE_INT_START    /* s2 = irq = index + EXCCODE_INT_START (64) */
  b         handle_irq                     /* a real interrupt, dispatch it */

.Lspurious_interrupt:
  move      $a0, $sp
  b         return_from_exception


handle_irq:
  /* Setup arg0(exception cause), arg1(context) */

  move      $a0, $s2
  move      $a1, $sp

  /* Call interrupt handler in C */

  bl        loongarch_doirq

return_from_exception:

  /* If context switch is needed, return a new sp */

  move      $sp, $a0

  REGLOAD   $s0, $sp, REG_EPC       /* restore epc */
  csrwr     $s0, LOONGARCH_CSR_ERA

  REGLOAD   $s0, $sp, REG_INT_CTX   /* restore status */
  csrwr     $s0, LOONGARCH_CSR_PRMD

  load_ctx  $sp

  REGLOAD   $sp, $sp, REG_SP        /* restore original sp */

  /* Return from exception */

  ertn
  .cfi_endproc

/*****************************************************************************
 *  Name: g_intstackalloc and g_intstacktop
 ****************************************************************************/

#if CONFIG_ARCH_INTERRUPTSTACK > 15
  .bss
  .balign 16
  .global g_intstackalloc
  .global g_intstacktop
  .type   g_intstackalloc, object
  .type   g_intstacktop, object
g_intstackalloc:
  .skip  STACKFRAME_ALIGN_UP(INT_STACK_SIZE)
g_intstacktop:
  .size  g_intstacktop, 0
  .size  g_intstackalloc, STACKFRAME_ALIGN_DOWN(INT_STACK_SIZE)
#endif
