/****************************************************************************
 * arch/loongarch/src/common/loongarch_fpu.S
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

#include <arch/arch.h>
#include <arch/csr.h>
#include <arch/irq.h>

#include "loongarch_macros.S"

#ifdef CONFIG_ARCH_FPU

/****************************************************************************
 * Public Symbols
 ****************************************************************************/

    .globl        loongarch_fpuconfig
    .globl        loongarch_savefpu
    .globl        loongarch_restorefpu
    .file         "loongarch_fpu.S"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: loongarch_fpuconfig
 *
 * Description:
 *   init fpu
 *
 * C Function Prototype:
 *   void loongarch_fpuconfig(void);
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   This function does not return anything explicitly.
 *
 ****************************************************************************/

    .type        loongarch_fpuconfig, function

loongarch_fpuconfig:
  li.w        $t0, CSR_EUEN_FPEN
  csrxchg     $t0, $t0, LOONGARCH_CSR_EUEN

  movgr2fcsr  $fcsr0, $zero
  ret

/****************************************************************************
 * Name: loongarch_savefpu
 *
 * Description:
 *   Given the pointer to a register save area (in A0), save the state of the
 *   floating point registers.
 *
 * C Function Prototype:
 *   void loongarch_savefpu(uintptr_t *regs, uintptr_t *fregs);
 *
 * Input Parameters:
 *   regs  - A pointer to the integer registers that contain the status
 *   fregs - A pointer to the register save area in which to save the
 *           floating point registers
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

    .type         loongarch_savefpu, function

loongarch_savefpu:

  loongarch_savefpu $a1

1:
  ret

/****************************************************************************
 * Name: loongarch_restorefpu
 *
 * Description:
 *   Given the pointer to a register save area (in A0), restore the state of
 *   the floating point registers.
 *
 * C Function Prototype:
 *   void loongarch_restorefpu(uintptr_t *regs, uintptr_t *fregs);
 *
 * Input Parameters:
 *   regs  - A pointer to the integer registers that contain the status
 *   fregs - A pointer to the register save area containing the floating
 *           point registers.
 *
 * Returned Value:
 *   This function does not return anything explicitly.  However, it is
 *   called from interrupt level assembly logic that assumes that r0 is
 *   preserved.
 *
 ****************************************************************************/

    .type        loongarch_restorefpu, function

loongarch_restorefpu:

  loongarch_loadfpu $a1

1:
  ret

#endif /* CONFIG_ARCH_FPU */
