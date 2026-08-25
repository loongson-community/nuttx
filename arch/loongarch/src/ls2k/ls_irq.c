/****************************************************************************
 * arch/loongarch/src/ls2k/ls_irq.c
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
#include <stdio.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include <arch/csr.h>

#include "loongarch_internal.h"
#include "ls.h"
#include "hardware/ls_memorymap.h"

/****************************************************************************
 * EIOINTC (Extended I/O Interrupt Controller)
 ****************************************************************************/

#define EIOINTC_VEC_COUNT          128
#define EIOINTC_IPMAP_COUNT        (EIOINTC_VEC_COUNT / 128)
#define EIOINTC_ENABLE_COUNT       (EIOINTC_VEC_COUNT / 32)
#define EIOINTC_ISR_COUNT          (EIOINTC_VEC_COUNT / 64)

#define EIOINTC_ENABLE_VEC(vector) \
  (EIOINTC_REG_ENABLE + ((vector / 32) * 4))

static void eiointc_init(void)
{
  uint32_t data;
  int i;

  uint64_t misc = iocsr_read64(LOONGARCH_IOCSR_MISC_FUNC);
  misc |= IOCSR_MISC_FUNC_EXT_IOI_EN;
  iocsr_write64(misc, LOONGARCH_IOCSR_MISC_FUNC);

  for (i = 0; i < EIOINTC_IPMAP_COUNT; i++)
    {
      data = (1U << 1) | ((1U << 1) << 8) |
             ((1U << 1) << 16) | ((1U << 1) << 24);
      iocsr_write32(data, EIOINTC_REG_IPMAP + i * 4);
    }
}

static int eiointc_dispatch(int irq, void *context, void *arg)
{
  uint64_t pending;
  int i;

  for (i = 0; i < EIOINTC_ISR_COUNT; i++)
    {
      pending = iocsr_read64(EIOINTC_REG_ISR + (i << 3));
      if (!pending)
        {
          continue;
        }

      iocsr_write64(pending, EIOINTC_REG_ISR + (i << 3));

      while (pending)
        {
          int bit = __builtin_ffsl(pending) - 1;
          int vector = bit + 64 * i;
          int ext_irq = LS_IRQ_EIOINTC_START + vector;

          irq_dispatch(ext_irq, (uintreg_t *)context);
          pending &= ~((uint64_t)1 << bit);
        }
    }

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void up_irqinitialize(void)
{
  up_irq_save();

  write_csr_ecfg(ECFGF(ECFGB_TIMER) | ECFGF(ECFGB_IP1));

  loongarch_exception_attach();

  eiointc_init();

  irq_attach(LOONGARCH_IRQ_IP1, eiointc_dispatch, NULL);

#if defined(CONFIG_STACK_COLORATION) && CONFIG_ARCH_INTERRUPTSTACK > 15
  size_t intstack_size = (CONFIG_ARCH_INTERRUPTSTACK & ~15);
  loongarch_stack_color(g_intstackalloc, intstack_size);
#endif

#ifndef CONFIG_SUPPRESS_INTERRUPTS
  loongarch_color_intstack();
  up_irq_enable();
#endif
}

void up_disable_irq(int irq)
{
  unsigned long ecfg;

  if (irq == LOONGARCH_IRQ_TIMER)
    {
      ecfg = csr_readq(LOONGARCH_CSR_ECFG);
      ecfg &= ~ECFGF(ECFGB_TIMER);
      csr_writeq(ecfg, LOONGARCH_CSR_ECFG);
    }
  else if (irq >= LS_IRQ_EIOINTC_START &&
           irq < LS_IRQ_EIOINTC_START + LS_IRQ_EIOINTC_COUNT)
    {
      int vector = irq - LS_IRQ_EIOINTC_START;
      uint32_t reg = EIOINTC_ENABLE_VEC(vector);
      uint32_t data = iocsr_read32(reg);
      data &= ~(1U << (vector % 32));
      iocsr_write32(data, reg);
    }
}

void up_enable_irq(int irq)
{
  unsigned long ecfg;

  if (irq == LOONGARCH_IRQ_TIMER)
    {
      ecfg = csr_readq(LOONGARCH_CSR_ECFG);
      ecfg |= ECFGF(ECFGB_TIMER);
      csr_writeq(ecfg, LOONGARCH_CSR_ECFG);
    }
  else if (irq >= LS_IRQ_EIOINTC_START &&
           irq < LS_IRQ_EIOINTC_START + LS_IRQ_EIOINTC_COUNT)
    {
      int vector = irq - LS_IRQ_EIOINTC_START;
      uint32_t reg = EIOINTC_ENABLE_VEC(vector);
      uint32_t data = iocsr_read32(reg);
      data |= (1U << (vector % 32));
      iocsr_write32(data, reg);
    }
}

void loongarch_ack_irq(int irq)
{
}
