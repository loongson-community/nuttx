/****************************************************************************
 * arch/loongarch/src/common/loongarch_internal.h
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

#ifndef __ARCH_LOONGARCH_SRC_COMMON_LOONGARCH_INTERNAL_H
#define __ARCH_LOONGARCH_SRC_COMMON_LOONGARCH_INTERNAL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <nuttx/compiler.h>
#  include <nuttx/sched.h>
#  include <sys/types.h>
#  include <stdint.h>
#  include <syscall.h>
#endif

#include <nuttx/irq.h>

#include "loongarch_common_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define FLOAD       __STR(fld.d)
#define FSTORE      __STR(fst.d)

#define REGLOAD     __STR(ld.d)
#define REGSTORE    __STR(st.d)

/* This is the value used to mark the stack for subsequent stack monitoring
 * logic.
 */

#define STACK_COLOR    0xdeadbeef
#define INTSTACK_COLOR 0xdeadbeef
#define HEAP_COLOR     'h'

#define STACK_FRAME_SIZE __XSTR(STACKFRAME_ALIGN)

/* Interrupt Stack macros */

#define INT_STACK_SIZE  (STACKFRAME_ALIGN_DOWN(CONFIG_ARCH_INTERRUPTSTACK))

/* Determine which (if any) console driver to use.  If a console is enabled
 * and no other console device is specified, then a serial console is
 * assumed.
 */

#ifndef CONFIG_DEV_CONSOLE
#  undef  USE_SERIALDRIVER
#  undef  USE_EARLYSERIALINIT
#else
#  if defined(CONFIG_CONSOLE_SYSLOG)
#    undef  USE_SERIALDRIVER
#    undef  USE_EARLYSERIALINIT
#  else
#    define USE_SERIALDRIVER 1
#    define USE_EARLYSERIALINIT 1
#  endif
#endif

#ifndef __ASSEMBLY__

static inline uint8_t getreg8(const volatile uintreg_t a)
{
  uint8_t v;
  __asm__ __volatile__("ld.b %0, %1, 0" : "=r" (v) : "r" (a));
  return v;
}

static inline void putreg8(uint8_t v, const volatile uintreg_t a)
{
  __asm__ __volatile__("st.b %0, %1, 0" : : "r" (v), "r" (a));
}

static inline uint16_t getreg16(const volatile uintreg_t a)
{
  uint16_t v;
  __asm__ __volatile__("ld.h %0, %1, 0" : "=r" (v) : "r" (a));
  return v;
}

static inline void putreg16(uint16_t v, const volatile uintreg_t a)
{
  __asm__ __volatile__("st.h %0, %1, 0" : : "r" (v), "r" (a));
}

static inline uint32_t getreg32(const volatile uintreg_t a)
{
  uint32_t v;
  __asm__ __volatile__("ld.w %0, %1, 0" : "=r" (v) : "r" (a));
  return v;
}

static inline void putreg32(uint32_t v, const volatile uintreg_t a)
{
  __asm__ __volatile__("st.w %0, %1, 0" : : "r" (v), "r" (a));
}

static inline uint64_t getreg64(const volatile uintreg_t a)
{
  uint64_t v;
  __asm__ __volatile__("ld.d %0, %1, 0" : "=r" (v) : "r" (a));
  return v;
}

static inline void putreg64(uint64_t v, const volatile uintreg_t a)
{
  __asm__ __volatile__("st.d %0, %1, 0" : : "r" (v), "r" (a));
}

#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__
/* Atomic modification of registers */

void modifyreg32(uintreg_t addr, uint32_t clearbits, uint32_t setbits);

/* Memory allocation ********************************************************/

#if CONFIG_MM_REGIONS > 1
void loongarch_addregion(void);
#else
#  define loongarch_addregion()
#endif

/* IRQ initialization *******************************************************/

void loongarch_ack_irq(int irq);

void loongarch_sigdeliver(void);
int loongarch_swint(int irq, void *context, void *arg);
uintptr_t loongarch_get_newintctx(void);
void loongarch_set_idleintctx(void);
void loongarch_exception_attach(void);

#ifdef CONFIG_ARCH_FPU
void loongarch_fpuconfig(void);
void loongarch_savefpu(uintreg_t *regs, uintreg_t *fregs);
void loongarch_restorefpu(uintreg_t *regs, uintreg_t *fregs);

/* Get FPU register save area */

static inline uintreg_t *loongarch_fpuregs(struct tcb_s *tcb)
{
  /* FPU registers are saved after the integer registers */

  return (uintreg_t *)((uintptr_t)tcb->xcp.regs + INT_XCPT_SIZE);
}
#else
#  define loongarch_fpuconfig()
#  define loongarch_savefpu(regs, fregs)
#  define loongarch_restorefpu(regs, fregs)
#  define loongarch_fpuregs(tcb)
#endif

/* Save / restore context of task */

static inline void loongarch_savecontext(struct tcb_s *tcb)
{
#ifdef CONFIG_ARCH_FPU
  /* Save current process FPU state to TCB */

  loongarch_savefpu(tcb->xcp.regs, loongarch_fpuregs(tcb));
#endif
}

static inline void loongarch_restorecontext(struct tcb_s *tcb)
{
#ifdef CONFIG_ARCH_FPU
  /* Restore FPU state for next process */

  loongarch_restorefpu(tcb->xcp.regs, loongarch_fpuregs(tcb));
#endif
}

/* Power management *********************************************************/

#ifdef CONFIG_PM
void loongarch_pminitialize(void);
#else
#  define loongarch_pminitialize()
#endif

/* DMA **********************************************************************/

#ifdef CONFIG_ARCH_DMA
void weak_function loongarch_dma_initialize(void);
#endif

/* SoC-specific CPU initialization ******************************************/

void weak_function loongarch_soc_initialize(void);

/* Low level serial output **************************************************/

void loongarch_lowputc(char ch);
void loongarch_lowputs(const char *str);

#ifdef USE_SERIALDRIVER
void loongarch_serialinit(void);
#endif

#ifdef USE_EARLYSERIALINIT
void loongarch_earlyserialinit(void);
#endif

/* Networking ***************************************************************/

/* Defined in board/xyz_network.c for board-specific Ethernet
 * implementations, or chip/xyx_ethernet.c for chip-specific Ethernet
 * implementations.
 */

#if defined(CONFIG_NET) && !defined(CONFIG_NETDEV_LATEINIT)
void loongarch_netinitialize(void);
#else
#  define loongarch_netinitialize()
#endif

/* Exception Handler ********************************************************/

uintreg_t *loongarch_doirq(int irq, uintreg_t *regs);
int loongarch_exception(int excode, void *regs, void *args);

/* Debug ********************************************************************/

#ifdef CONFIG_STACK_COLORATION
size_t loongarch_stack_check(uintptr_t alloc, size_t size);
void loongarch_stack_color(void *stackbase, size_t nbytes);
#endif

#if defined(CONFIG_STACK_COLORATION) && \
    defined(CONFIG_ARCH_INTERRUPTSTACK) && CONFIG_ARCH_INTERRUPTSTACK > 15
void loongarch_color_intstack(void);
#else
#  define loongarch_color_intstack()
#endif

#ifdef CONFIG_SMP
void loongarch_cpu_boot(int cpu);
int loongarch_smp_call_handler(int irq, void *c, void *arg);
void loongarch_timer_secondary_init(void);
#endif

/****************************************************************************
 * Name: loongarch_jump_to_user
 *
 * Description:
 *   Routine to jump to user space, called when a user process is started and
 *   the kernel is ready to give control to the user task in user space.
 *
 * Parameters:
 *   entry - Process entry point.
 *   a0    - Parameter 0 for the process.
 *   a1    - Parameter 1 for the process.
 *   a2    - Parameter 2 for the process.
 *   sp    - User stack pointer.
 *   regs  - Integer register save area to use.
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void loongarch_jump_to_user(uintptr_t entry, uintptr_t arg1, uintptr_t arg2,
                            uintptr_t arg3, uintptr_t sp,
                            uintreg_t *regs) noreturn_function;

/* Context switching via system calls ***************************************/

/****************************************************************************
 * Name: loongarch_fullcontextrestore
 *
 * Description:
 *   Restores the full context.
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#define loongarch_fullcontextrestore() \
  do                                   \
    {                                  \
      sys_call0(SYS_restore_context);  \
    }                                  \
  while (1)

/****************************************************************************
 * Name: loongarch_switchcontext
 *
 * Description:
 *   Switches the context.
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#define loongarch_switchcontext()    \
  do                                 \
    {                                \
      sys_call0(SYS_switch_context); \
    }                                \
  while (0)

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif /* __ASSEMBLY__ */

#endif /* __ARCH_LOONGARCH_SRC_COMMON_LOONGARCH_INTERNAL_H */
