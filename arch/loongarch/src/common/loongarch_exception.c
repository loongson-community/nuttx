/****************************************************************************
 * arch/loongarch/src/common/loongarch_exception.c
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
#include <assert.h>

#include <nuttx/debug.h>
#include <nuttx/irq.h>
#include <nuttx/arch.h>

#include "sched/sched.h"
#include "loongarch_internal.h"
#include "chip.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const char *g_reasons_str[LOONGARCH_IRQ_GCSC + 1] =
{
  "INT (Interrupt)",
  "TLB miss on load",
  "TLB miss on store",
  "TLB miss on ifetch",
  "TLB modified fault",
  "TLB Read-Inhibit",
  "TLB Execution-Inhibit",
  "TLB Privilege Error",
  "Address Error",
  "Unalign Access",
  "Out of bounds",
  "System call",
  "Breakpoint",
  "Inst. Not Exist",
  "Inst. Privileged Error",
  "FPU Disabled",
  "LSX Disabled",
  "LASX Disabled",
  "Floating Point Exception",
  "WPEF/WPEM",
  "BTD",
  "BTE",
  "GSPR",
  "HVC",
  "GCSC / GCHC",
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: loongarch_fault_handler
 *
 * Description:
 *   Handle a fault caused by the running task. If the task is a user task
 *   not currently in a syscall or interrupt context, kill it with SIGSEGV
 *   instead of taking down the whole system. Otherwise (kernel thread, a
 *   fault while already in kernel context on behalf of a syscall, or a
 *   fault while handling an interrupt) there is no safe task to kill, so
 *   panic.
 *
 * Input Parameters:
 *   excode - The (masked) machine excode of the exception, used for the
 *     panic message only.
 *   regs  - A pointer to the register state at the time of the exception.
 *
 ****************************************************************************/

static void loongarch_fault_handler(uintreg_t excode, void *regs)
{
  _alert("PANIC!!! Exception = %" PRIxREG "\n", excode);
  up_irq_save();
  up_set_interrupt_context(true);
  PANIC_WITH_REGS("panic", regs);
}

/****************************************************************************
 * Name: loongarch_exception
 *
 * Description:
 *   This is the exception handler.
 *
 ****************************************************************************/

int loongarch_exception(int excode, void *regs, void *args)
{
  uintreg_t era = csr_readq(LOONGARCH_CSR_ERA);

  _alert("EXCEPTION: %s. ExCode: %" PRIxREG ", ERA: %" PRIxREG "\n",
         excode > LOONGARCH_IRQ_GCSC ? "Unknown" : g_reasons_str[excode],
         excode, era);

  loongarch_fault_handler(excode, regs);

  return 0;
}

/****************************************************************************
 * Name: loongarch_exception_attach
 *
 * Description:
 *   Attach standard exception with suitable handler
 *
 ****************************************************************************/

void loongarch_exception_attach(void)
{
  int i;

  for (i = LOONGARCH_IRQ_TLBL; i <= LOONGARCH_MAX_EXCEPTION; i++)
    {
      if (i != LOONGARCH_IRQ_SYS)
        {
          irq_attach(i, loongarch_exception, NULL);
        }
    }

  /* Attach the syscall interrupt handler */

  irq_attach(LOONGARCH_IRQ_SYS, loongarch_swint, NULL);
}
