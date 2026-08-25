/****************************************************************************
 * arch/loongarch/src/common/loongarch_saveusercontext.S
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

#include "loongarch_macros.S"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Symbols
 ****************************************************************************/

/****************************************************************************
 * Name: up_saveusercontext
 *
 * Description:
 *   Save the current thread context
 *
 ****************************************************************************/

  .section .text
  .global up_saveusercontext
  .align  8

up_saveusercontext:

  save_ctx   $a0

  csrrd      $t0, LOONGARCH_CSR_PRMD
  REGSTORE   $t0, $a0, REG_INT_CTX    /* PRMD */

  REGSTORE   $sp, $a0, REG_SP         /* original SP */

  csrrd      $t0, LOONGARCH_CSR_ERA
  REGSTORE   $t0, $a0, REG_EPC

#ifdef CONFIG_ARCH_FPU
  addi.d     $a0, $a0, INT_XCPT_SIZE  /* Save FPU after integer regs */
  loongarch_savefpu $a0
#endif

  li.d       $a0, 0
  jr         $ra

  .size	up_saveusercontext, . - up_saveusercontext
  .end
