/****************************************************************************
 * arch/loongarch/include/irq.h
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

/* This file should never be included directly but, rather, only indirectly
 * through nuttx/irq.h
 */

#ifndef __ARCH_LOONGARCH_INCLUDE_IRQ_H
#define __ARCH_LOONGARCH_INCLUDE_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

/* Include chip-specific IRQ definitions (including IRQ numbers) */

#include <nuttx/config.h>

#include <sys/types.h>

#include <arch/csr.h>
#include <arch/chip/irq.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef __ASSEMBLY__
#  define __STR(s)  s
#else
#  define __STR(s)  #s
#endif
#define __XSTR(s)   __STR(s)

/* LoongArch requires a 16-byte stack alignment. */

#define STACKFRAME_ALIGN 16

/****************************************************************************
 * Map LoongArch exception code to NuttX IRQ,
 * the exception that code > 24 is reserved or custom exception.
 *
 * The content of vector table:
 *
 * |             IRQ               |             Comments               |
 * |:-----------------------------:|:----------------------------------:|
 * |              0                |             Reserved               |
 * |              1                |          TLB Load Miss             |
 * |              1                |          TLB Store Miss            |
 * |             ...               |         Other exceptions           |
 * |    LOONGARCH_MAX_EXCEPTION    |  The IRQ number of last exception  |
 * |  LOONGARCH_MAX_EXCEPTION + 1  |  The IRQ number of first interrupt |
 * |  LOONGARCH_MAX_EXCEPTION + 2  | The IRQ number of second interrupt |
 * | LOONGARCH_MAX_EXCEPTION + xxx |   The IRQ number of xxx interrupt  |
 *
 * And please provide the definition of custom exception if exists:
 * #define LOONGARCH_CUSTOM_EXCEPTION_REASONS  \
 *    "Custom exception1", \
 *    "Custom exception2",
 *
 ****************************************************************************/

/* IRQ 0-LOONGARCH_MAX_EXCEPTION : */

#define LOONGARCH_IRQ_RSV       (0)   /* Reserved */
#define LOONGARCH_IRQ_TLBL      (1)   /* TLB Load Miss */
#define LOONGARCH_IRQ_TLBS      (2)   /* TLB Store Miss */
#define LOONGARCH_IRQ_TLBI      (3)   /* TLB Instruction Fetch Miss */
#define LOONGARCH_IRQ_TLBM      (4)   /* TLB Modify Exception */
#define LOONGARCH_IRQ_TLBRI     (5)   /* TLB Read Invalid */
#define LOONGARCH_IRQ_TLBXI     (6)   /* TLB Execute Invalid */
#define LOONGARCH_IRQ_TLBPE     (7)   /* TLB Permission Error */
#define LOONGARCH_IRQ_ADE       (8)   /* Address Error (Data) */
#define LOONGARCH_IRQ_ALE       (9)   /* Address Alignment Error */
#define LOONGARCH_IRQ_OOB       (10)  /* Out of Bounds */
#define LOONGARCH_IRQ_SYS       (11)  /* System Call */
#define LOONGARCH_IRQ_BP        (12)  /* Breakpoint */
#define LOONGARCH_IRQ_INE       (13)  /* Instruction Not Exist */
#define LOONGARCH_IRQ_IPE       (14)  /* Instruction Privilege Error */
#define LOONGARCH_IRQ_FPDIS     (15)  /* FPU Disabled */
#define LOONGARCH_IRQ_LSXDIS    (16)  /* LSX Disabled */
#define LOONGARCH_IRQ_LASXDIS   (17)  /* LASX Disabled */
#define LOONGARCH_IRQ_FPE       (18)  /* FPU Exception */
#define LOONGARCH_IRQ_WPE       (19)  /* WPEF / WPEM */
#define LOONGARCH_IRQ_BTD       (20)  /* BTD */
#define LOONGARCH_IRQ_BTE       (21)  /* BTE */
#define LOONGARCH_IRQ_GSPR      (22)  /* GSPR */
#define LOONGARCH_IRQ_HVC       (23)  /* HVC */
#define LOONGARCH_IRQ_GCSC      (24)  /* GCSC / GCHC */

/* Keep origin definition here for compatibility */

#ifndef LOONGARCH_MAX_EXCEPTION
#  define LOONGARCH_MAX_EXCEPTION (63)
#endif

/* IRQ (LOONGARCH_MAX_EXCEPTION + 1)- : (async event:interrupt=1) */

#define LOONGARCH_IRQ_ASYNC     (LOONGARCH_MAX_EXCEPTION + 1)
#define LOONGARCH_IRQ_SIP0      (LOONGARCH_IRQ_ASYNC + 0)   /* Software Interrupt 0 */
#define LOONGARCH_IRQ_SIP1      (LOONGARCH_IRQ_ASYNC + 1)   /* Software Interrupt 1 */
#define LOONGARCH_IRQ_IP0       (LOONGARCH_IRQ_ASYNC + 2)   /* Hardware Interrupt 0 */
#define LOONGARCH_IRQ_IP1       (LOONGARCH_IRQ_ASYNC + 3)   /* Hardware Interrupt 1 */
#define LOONGARCH_IRQ_IP2       (LOONGARCH_IRQ_ASYNC + 4)   /* Hardware Interrupt 2 */
#define LOONGARCH_IRQ_IP3       (LOONGARCH_IRQ_ASYNC + 5)   /* Hardware Interrupt 3 */
#define LOONGARCH_IRQ_IP4       (LOONGARCH_IRQ_ASYNC + 6)   /* Hardware Interrupt 4 */
#define LOONGARCH_IRQ_IP5       (LOONGARCH_IRQ_ASYNC + 7)   /* Hardware Interrupt 5 */
#define LOONGARCH_IRQ_IP6       (LOONGARCH_IRQ_ASYNC + 8)   /* Hardware Interrupt 6 */
#define LOONGARCH_IRQ_IP7       (LOONGARCH_IRQ_ASYNC + 9)   /* Hardware Interrupt 7 */
#define LOONGARCH_IRQ_PC        (LOONGARCH_IRQ_ASYNC + 10)  /* Performance Counter */
#define LOONGARCH_IRQ_TIMER     (LOONGARCH_IRQ_ASYNC + 11)  /* Timer */
#define LOONGARCH_IRQ_IPI       (LOONGARCH_IRQ_ASYNC + 12)  /* Inter-Processor Interrupt */

#define LOONGARCH_MAX_IRQ       LOONGARCH_IRQ_IPI

/* Configuration ************************************************************/

/* Processor PC */

#define REG_EPC_NDX         0

/* General purpose registers
 * $0: Zero register does not need to be saved
 * $1: ra (return address)
 */

#define REG_R1_NDX          1

/* $2 = tp: Thread pointer */

#define REG_R2_NDX          2

/* $3 = sp: Stack pointer */

#define REG_R3_NDX          3

/* $4-$5 = a0-a1: Argument registers / Return registers
 * $6-$11 = a2-a7: Argument registers
 */

#define REG_R4_NDX          4
#define REG_R5_NDX          5
#define REG_R6_NDX          6
#define REG_R7_NDX          7
#define REG_R8_NDX          8
#define REG_R9_NDX          9
#define REG_R10_NDX         10
#define REG_R11_NDX         11

/* $12-$20 = t0-t8: Temp registers */

#define REG_R12_NDX         12
#define REG_R13_NDX         13
#define REG_R14_NDX         14
#define REG_R15_NDX         15
#define REG_R16_NDX         16
#define REG_R17_NDX         17
#define REG_R18_NDX         18
#define REG_R19_NDX         19
#define REG_R20_NDX         20

/* $21 Reserved */

#define REG_R21_NDX         21

/* $22 = fp: Frame pointer */

#define REG_R22_NDX         22

/* $23-31 = s0-s8: Static registers */

#define REG_R23_NDX         23
#define REG_R24_NDX         24
#define REG_R25_NDX         25
#define REG_R26_NDX         26
#define REG_R27_NDX         27
#define REG_R28_NDX         28
#define REG_R29_NDX         29
#define REG_R30_NDX         30
#define REG_R31_NDX         31

/* Interrupt Context register */

#define REG_INT_CTX_NDX     32

#define INT_XCPT_REGS       (REG_INT_CTX_NDX + 1)

#define INT_REG_SIZE        8

#define INT_XCPT_SIZE       (INT_REG_SIZE * INT_XCPT_REGS)

#define FPU_REG_SIZE        1

#ifdef CONFIG_ARCH_FPU
#  define REG_F0_NDX        (FPU_REG_SIZE * 0)
#  define REG_F1_NDX        (FPU_REG_SIZE * 1)
#  define REG_F2_NDX        (FPU_REG_SIZE * 2)
#  define REG_F3_NDX        (FPU_REG_SIZE * 3)
#  define REG_F4_NDX        (FPU_REG_SIZE * 4)
#  define REG_F5_NDX        (FPU_REG_SIZE * 5)
#  define REG_F6_NDX        (FPU_REG_SIZE * 6)
#  define REG_F7_NDX        (FPU_REG_SIZE * 7)
#  define REG_F8_NDX        (FPU_REG_SIZE * 8)
#  define REG_F9_NDX        (FPU_REG_SIZE * 9)
#  define REG_F10_NDX       (FPU_REG_SIZE * 10)
#  define REG_F11_NDX       (FPU_REG_SIZE * 11)
#  define REG_F12_NDX       (FPU_REG_SIZE * 12)
#  define REG_F13_NDX       (FPU_REG_SIZE * 13)
#  define REG_F14_NDX       (FPU_REG_SIZE * 14)
#  define REG_F15_NDX       (FPU_REG_SIZE * 15)
#  define REG_F16_NDX       (FPU_REG_SIZE * 16)
#  define REG_F17_NDX       (FPU_REG_SIZE * 17)
#  define REG_F18_NDX       (FPU_REG_SIZE * 18)
#  define REG_F19_NDX       (FPU_REG_SIZE * 19)
#  define REG_F20_NDX       (FPU_REG_SIZE * 20)
#  define REG_F21_NDX       (FPU_REG_SIZE * 21)
#  define REG_F22_NDX       (FPU_REG_SIZE * 22)
#  define REG_F23_NDX       (FPU_REG_SIZE * 23)
#  define REG_F24_NDX       (FPU_REG_SIZE * 24)
#  define REG_F25_NDX       (FPU_REG_SIZE * 25)
#  define REG_F26_NDX       (FPU_REG_SIZE * 26)
#  define REG_F27_NDX       (FPU_REG_SIZE * 27)
#  define REG_F28_NDX       (FPU_REG_SIZE * 28)
#  define REG_F29_NDX       (FPU_REG_SIZE * 29)
#  define REG_F30_NDX       (FPU_REG_SIZE * 30)
#  define REG_F31_NDX       (FPU_REG_SIZE * 31)
#  define REG_FCSR_NDX      (FPU_REG_SIZE * 32)
#  define REG_FCC_NDX       (FPU_REG_SIZE * 33)

#  define FPU_XCPT_REGS     (FPU_REG_SIZE * 34)
#  define FPU_XCPT_SIZE     (INT_REG_SIZE * FPU_XCPT_REGS)
#else /* !CONFIG_ARCH_FPU */
#  define FPU_XCPT_REGS     (0)
#  define FPU_XCPT_SIZE     (0)
#endif /* CONFIG_ARCH_FPU */

#define XCPTCONTEXT_REGS    (INT_XCPT_REGS + FPU_XCPT_REGS)

#ifdef CONFIG_ARCH_LAZYFPU
/* Save only integer regs. FPU is handled separately */

#define XCPTCONTEXT_SIZE    (INT_XCPT_SIZE)
#else
/* Save FPU registers with the integer registers */

#define XCPTCONTEXT_SIZE    (INT_XCPT_SIZE + FPU_XCPT_SIZE)
#endif

/* In assembly language, values have to be referenced as byte address
 * offsets.  But in C, it is more convenient to reference registers as
 * register save table offsets.
 */

#ifdef __ASSEMBLY__
#  define REG_EPC           (INT_REG_SIZE*REG_EPC_NDX)
#  define REG_R1            (INT_REG_SIZE*REG_R1_NDX)
#  define REG_R2            (INT_REG_SIZE*REG_R2_NDX)
#  define REG_R3            (INT_REG_SIZE*REG_R3_NDX)
#  define REG_R4            (INT_REG_SIZE*REG_R4_NDX)
#  define REG_R5            (INT_REG_SIZE*REG_R5_NDX)
#  define REG_R6            (INT_REG_SIZE*REG_R6_NDX)
#  define REG_R7            (INT_REG_SIZE*REG_R7_NDX)
#  define REG_R8            (INT_REG_SIZE*REG_R8_NDX)
#  define REG_R9            (INT_REG_SIZE*REG_R9_NDX)
#  define REG_R10           (INT_REG_SIZE*REG_R10_NDX)
#  define REG_R11           (INT_REG_SIZE*REG_R11_NDX)
#  define REG_R12           (INT_REG_SIZE*REG_R12_NDX)
#  define REG_R13           (INT_REG_SIZE*REG_R13_NDX)
#  define REG_R14           (INT_REG_SIZE*REG_R14_NDX)
#  define REG_R15           (INT_REG_SIZE*REG_R15_NDX)
#  define REG_R16           (INT_REG_SIZE*REG_R16_NDX)
#  define REG_R17           (INT_REG_SIZE*REG_R17_NDX)
#  define REG_R18           (INT_REG_SIZE*REG_R18_NDX)
#  define REG_R19           (INT_REG_SIZE*REG_R19_NDX)
#  define REG_R20           (INT_REG_SIZE*REG_R20_NDX)
#  define REG_R21           (INT_REG_SIZE*REG_R21_NDX)
#  define REG_R22           (INT_REG_SIZE*REG_R22_NDX)
#  define REG_R23           (INT_REG_SIZE*REG_R23_NDX)
#  define REG_R24           (INT_REG_SIZE*REG_R24_NDX)
#  define REG_R25           (INT_REG_SIZE*REG_R25_NDX)
#  define REG_R26           (INT_REG_SIZE*REG_R26_NDX)
#  define REG_R27           (INT_REG_SIZE*REG_R27_NDX)
#  define REG_R28           (INT_REG_SIZE*REG_R28_NDX)
#  define REG_R29           (INT_REG_SIZE*REG_R29_NDX)
#  define REG_R30           (INT_REG_SIZE*REG_R30_NDX)
#  define REG_R31           (INT_REG_SIZE*REG_R31_NDX)
#  define REG_INT_CTX       (INT_REG_SIZE*REG_INT_CTX_NDX)

#ifdef CONFIG_ARCH_FPU
#  define REG_F0            (INT_REG_SIZE*REG_F0_NDX)
#  define REG_F1            (INT_REG_SIZE*REG_F1_NDX)
#  define REG_F2            (INT_REG_SIZE*REG_F2_NDX)
#  define REG_F3            (INT_REG_SIZE*REG_F3_NDX)
#  define REG_F4            (INT_REG_SIZE*REG_F4_NDX)
#  define REG_F5            (INT_REG_SIZE*REG_F5_NDX)
#  define REG_F6            (INT_REG_SIZE*REG_F6_NDX)
#  define REG_F7            (INT_REG_SIZE*REG_F7_NDX)
#  define REG_F8            (INT_REG_SIZE*REG_F8_NDX)
#  define REG_F9            (INT_REG_SIZE*REG_F9_NDX)
#  define REG_F10           (INT_REG_SIZE*REG_F10_NDX)
#  define REG_F11           (INT_REG_SIZE*REG_F11_NDX)
#  define REG_F12           (INT_REG_SIZE*REG_F12_NDX)
#  define REG_F13           (INT_REG_SIZE*REG_F13_NDX)
#  define REG_F14           (INT_REG_SIZE*REG_F14_NDX)
#  define REG_F15           (INT_REG_SIZE*REG_F15_NDX)
#  define REG_F16           (INT_REG_SIZE*REG_F16_NDX)
#  define REG_F17           (INT_REG_SIZE*REG_F17_NDX)
#  define REG_F18           (INT_REG_SIZE*REG_F18_NDX)
#  define REG_F19           (INT_REG_SIZE*REG_F19_NDX)
#  define REG_F20           (INT_REG_SIZE*REG_F20_NDX)
#  define REG_F21           (INT_REG_SIZE*REG_F21_NDX)
#  define REG_F22           (INT_REG_SIZE*REG_F22_NDX)
#  define REG_F23           (INT_REG_SIZE*REG_F23_NDX)
#  define REG_F24           (INT_REG_SIZE*REG_F24_NDX)
#  define REG_F25           (INT_REG_SIZE*REG_F25_NDX)
#  define REG_F26           (INT_REG_SIZE*REG_F26_NDX)
#  define REG_F27           (INT_REG_SIZE*REG_F27_NDX)
#  define REG_F28           (INT_REG_SIZE*REG_F28_NDX)
#  define REG_F29           (INT_REG_SIZE*REG_F29_NDX)
#  define REG_F30           (INT_REG_SIZE*REG_F30_NDX)
#  define REG_F31           (INT_REG_SIZE*REG_F31_NDX)
#  define REG_FCSR          (INT_REG_SIZE*REG_FCSR_NDX)
#  define REG_FCC           (INT_REG_SIZE*REG_FCC_NDX)
#endif /* CONFIG_ARCH_FPU */

#else
#  define REG_EPC           REG_EPC_NDX
#  define REG_R1            REG_R1_NDX
#  define REG_R2            REG_R2_NDX
#  define REG_R3            REG_R3_NDX
#  define REG_R4            REG_R4_NDX
#  define REG_R5            REG_R5_NDX
#  define REG_R6            REG_R6_NDX
#  define REG_R7            REG_R7_NDX
#  define REG_R8            REG_R8_NDX
#  define REG_R9            REG_R9_NDX
#  define REG_R10           REG_R10_NDX
#  define REG_R11           REG_R11_NDX
#  define REG_R12           REG_R12_NDX
#  define REG_R13           REG_R13_NDX
#  define REG_R14           REG_R14_NDX
#  define REG_R15           REG_R15_NDX
#  define REG_R16           REG_R16_NDX
#  define REG_R17           REG_R17_NDX
#  define REG_R18           REG_R18_NDX
#  define REG_R19           REG_R19_NDX
#  define REG_R20           REG_R20_NDX
#  define REG_R21           REG_R21_NDX
#  define REG_R22           REG_R22_NDX
#  define REG_R23           REG_R23_NDX
#  define REG_R24           REG_R24_NDX
#  define REG_R25           REG_R25_NDX
#  define REG_R26           REG_R26_NDX
#  define REG_R27           REG_R27_NDX
#  define REG_R28           REG_R28_NDX
#  define REG_R29           REG_R29_NDX
#  define REG_R30           REG_R30_NDX
#  define REG_R31           REG_R31_NDX
#  define REG_INT_CTX       REG_INT_CTX_NDX

#ifdef CONFIG_ARCH_FPU
#  define REG_F0            REG_F0_NDX
#  define REG_F1            REG_F1_NDX
#  define REG_F2            REG_F2_NDX
#  define REG_F3            REG_F3_NDX
#  define REG_F4            REG_F4_NDX
#  define REG_F5            REG_F5_NDX
#  define REG_F6            REG_F6_NDX
#  define REG_F7            REG_F7_NDX
#  define REG_F8            REG_F8_NDX
#  define REG_F9            REG_F9_NDX
#  define REG_F10           REG_F10_NDX
#  define REG_F11           REG_F11_NDX
#  define REG_F12           REG_F12_NDX
#  define REG_F13           REG_F13_NDX
#  define REG_F14           REG_F14_NDX
#  define REG_F15           REG_F15_NDX
#  define REG_F16           REG_F16_NDX
#  define REG_F17           REG_F17_NDX
#  define REG_F18           REG_F18_NDX
#  define REG_F19           REG_F19_NDX
#  define REG_F20           REG_F20_NDX
#  define REG_F21           REG_F21_NDX
#  define REG_F22           REG_F22_NDX
#  define REG_F23           REG_F23_NDX
#  define REG_F24           REG_F24_NDX
#  define REG_F25           REG_F25_NDX
#  define REG_F26           REG_F26_NDX
#  define REG_F27           REG_F27_NDX
#  define REG_F28           REG_F28_NDX
#  define REG_F29           REG_F29_NDX
#  define REG_F30           REG_F30_NDX
#  define REG_F31           REG_F31_NDX
#  define REG_FCSR          REG_FCSR_NDX
#  define REG_FCC           REG_FCC_NDX
#endif /* CONFIG_ARCH_FPU */

#endif /* __ASSEMBLY__ */

/* Now define more user friendly alternative name that can be used either
 * in assembly or C contexts.
 */

/* $1 = ra: Return address */

#define REG_RA              REG_R1

/* $2 = tp: Thread pointer */

#define REG_TP              REG_R2

/* $3 = sp: Stack pointer */

#define REG_SP              REG_R3

/* $4-$5 = a0-a1: Argument / Return registers */

#define REG_A0              REG_R4
#define REG_A1              REG_R5

/* $6-$11 = a2-a7: Argument registers */

#define REG_A2              REG_R6
#define REG_A3              REG_R7
#define REG_A4              REG_R8
#define REG_A5              REG_R9
#define REG_A6              REG_R10
#define REG_A7              REG_R11

/* $12-$20 = t0-t8: Temporary registers */

#define REG_T0              REG_R12
#define REG_T1              REG_R13
#define REG_T2              REG_R14
#define REG_T3              REG_R15
#define REG_T4              REG_R16
#define REG_T5              REG_R17
#define REG_T6              REG_R18
#define REG_T7              REG_R19
#define REG_T8              REG_R20

/* $22 = fp: Frame pointer */

#define REG_FP              REG_R22

/* $23-$31 = s0-s8: Static / Saved registers */

#define REG_S0              REG_R23
#define REG_S1              REG_R24
#define REG_S2              REG_R25
#define REG_S3              REG_R26
#define REG_S4              REG_R27
#define REG_S5              REG_R28
#define REG_S6              REG_R29
#define REG_S7              REG_R30
#define REG_S8              REG_R31

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef __ASSEMBLY__

/* The following structure is included in the TCB and defines the complete
 * state of the thread.
 */

struct xcptcontext
{
  uintreg_t *saved_regs;

  /* Integer register save area */

  uintreg_t *regs;

  /* FPU register save area */

#if defined(CONFIG_ARCH_FPU) && defined(CONFIG_ARCH_LAZYFPU)
  uintreg_t fregs[FPU_XCPT_REGS];
#endif
};

#endif /* __ASSEMBLY__ */

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************
 * Inline functions
 ****************************************************************************/

/* Return the current value of the stack pointer */

static inline_function uintptr_t up_getsp(void)
{
  register uintptr_t sp;
  __asm__ __volatile__
    (
      "move %0, $sp\n"
      : "=r"(sp)
    );
  return sp;
}

/****************************************************************************
 * Public Data
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/* g_interrupt_context store irq status */

EXTERN volatile bool g_interrupt_context[CONFIG_SMP_NCPUS];

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_irq_save
 *
 * Description:
 *   Disable IRQs and return the previous IRQ state
 *
 ****************************************************************************/

noinstrument_function static inline_function irqstate_t up_irq_save(void)
{
  irqstate_t flags;

  __asm__ __volatile__
    (
      "csrrd   %0, %2\n\t"
      "csrxchg $zero, %1, %2\n\t"
      : "=&r"(flags)
      : "r"(CSR_CRMD_IE), "i"(LOONGARCH_CSR_CRMD)
      : "memory"
    );

  return flags & CSR_CRMD_IE;
}

/****************************************************************************
 * Name: up_irq_restore
 *
 * Description:
 *   Restore saved IRQ state
 *
 ****************************************************************************/

noinstrument_function static inline_function
void up_irq_restore(irqstate_t flags)
{
  __asm__ __volatile__
    (
      "csrxchg %0, %1, %2\n\t"
      : "+r"(flags)
      : "r"(CSR_CRMD_IE), "i"(LOONGARCH_CSR_CRMD)
      : "memory"
    );
}

/****************************************************************************
 * Name: up_irq_enable
 *
 * Description:
 *   Enable IRQs and return the previous IRQ state
 *
 ****************************************************************************/

noinstrument_function
static inline_function irqstate_t up_irq_enable(void)
{
  irqstate_t flags;

  __asm__ __volatile__
    (
      "csrrd   %0, %2\n\t"
      "csrxchg %1, %1, %2\n\t"
      : "=&r"(flags)
      : "r"(CSR_CRMD_IE), "i"(LOONGARCH_CSR_CRMD)
      : "memory"
    );

  return flags & CSR_CRMD_IE;
}

/****************************************************************************
 * Name: up_set_interrupt_context
 *
 * Description:
 *   Set the interrupt handler context.
 *
 ****************************************************************************/

noinstrument_function
static inline_function void up_set_interrupt_context(bool flag)
{
  g_interrupt_context[0] = flag;
}

/****************************************************************************
 * Name: up_interrupt_context
 *
 * Description:
 *   Return true is we are currently executing in the interrupt
 *   handler context.
 *
 ****************************************************************************/

noinstrument_function static inline_function bool up_interrupt_context(void)
{
  return g_interrupt_context[0];
}

/****************************************************************************
 * Name: up_getusrpc
 ****************************************************************************/

#define up_getusrpc(regs) \
    (((uintptr_t *)((regs) ? (regs) : running_regs()))[REG_EPC])

/****************************************************************************
 * Name: up_getusrsp
 ****************************************************************************/

#define up_getusrsp(regs) \
    (((uintptr_t*)(regs))[REG_SP])

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */

#endif /* __ARCH_LOONGARCH_INCLUDE_IRQ_H */
