/****************************************************************************
 * arch/loongarch/src/ls2k/ls_head.S
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

#include <arch/irq.h>
#include <arch/csr.h>

#include "chip.h"
#include "loongarch_internal.h"
#include "hardware/ls_memorymap.h"
#include "loongarch_macros.S"

/****************************************************************************
 * Pre-processor Definitions
 *
 * Additional constants not in ls_memorymap.h
 ****************************************************************************/

#define BOOT_SPACE_BASE         0x1c000000

#define LS_IDLESTACK_TOP   (CACHED_MEMORY_ADDR | LS_L2CACHE_BASE | LS_L2CACHE_SIZE)

#define EARLY_UART_BASE         (UNCACHED_MEMORY_ADDR | 0x16100000)
#define EARLY_UART_THR          (EARLY_UART_BASE + 0x00)
#define EARLY_UART_LSR          (EARLY_UART_BASE + 0x05)
#define EARLY_UART_LSR_THRE     (1 << 5)

  .macro EARLY_UART_INIT
  li.d      $t2, EARLY_UART_BASE
  li.w      $t3, 0x80
  st.b      $t3, $t2, 3
  li.w      $t3, 108
  st.b      $t3, $t2, 0
  li.w      $t3, 0
  st.b      $t3, $t2, 1
  li.w      $t3, 0x03
  st.b      $t3, $t2, 3
  li.w      $t3, 0x07
  st.b      $t3, $t2, 2
  .endm

  .macro EARLY_PUTC ch
  li.d      $t2, EARLY_UART_LSR
1:
  ld.b      $t3, $t2, 0
  andi      $t3, $t3, EARLY_UART_LSR_THRE
  beqz      $t3, 1b
  li.d      $t2, EARLY_UART_THR
  li.w      $t3, \ch
  st.b      $t3, $t2, 0
  .endm

/****************************************************************************
 * Public Symbols
 ****************************************************************************/

  .section .text.__start, "ax", @progbits
  .global __start
  .type  __start, function

__start:

  /* CPUCFG 0x0 PRID 处理器标识 */
  li.d      $t0, (1 << (24 + 32))
  csrxchg   $t0, $t0, LOONGARCH_CSR_PRID

  /* CPUCFG 0x13 Index-log2 配置字 10 中 L2 IU Present 对应的 Cache
   * log2(每一路 Cache 行数) = 0xa
   */
  li.d      $t0, (0xa << (16 + 32))
  li.d      $t1, (0xff << (16 + 32))
  csrxchg   $t0, $t1, LOONGARCH_CSR_MCSR9

  /* PLV0 特权等级下该性能监测器的计数使能。1——进行计数，0——停止计数。
   * enable perf counter as cp0 counter
   */
  li.w      $t0, (0x1 << 16)
  csrxchg   $t0, $t0, LOONGARCH_CSR_PERFCTRL0

  /* CPUCFG 0x5 IMPCTL1 位 9: auto flush SFB; 位 12: fast_ldq_dis
   * open auto flush SFB
   */
  li.w      $t0, (0x1 << 9)
  csrxchg   $t0, $t0, LOONGARCH_CSR_IMPCTL1

  /* close fast_ldq_dis */
  li.w      $t0, (0x1 << 12)
  csrxchg   $zero, $t0, LOONGARCH_CSR_IMPCTL1

  /* CPUCFG 0x18 MCSR24: low power setting */
  li.w      $t0, 0xe
  csrxchg   $t0, $t0, LOONGARCH_CSR_MCSR24

  /* set DMW0(0x180) to uncached, set DMW1(0x181) to cached
   * DMW0: 强序非缓存，PLV0-3 可用，0x8000 0000 0000 0000 - 0x8000 FFFF FFFF FFFF (PALEN = 48)
   */
  li.d      $t0, UNCACHED_MEMORY_ADDR | 0xf
  csrwr     $t0, LOONGARCH_CSR_DMWIN0
  /* DMW1: 一致可缓存，PLV0-3 可用，0x9000 0000 0000 0000 - 0x9000 FFFF FFFF FFFF (PALEN = 48) */
  li.d      $t0, CACHED_MEMORY_ADDR | 0x1f
  csrwr     $t0, LOONGARCH_CSR_DMWIN1

	/* CSR ECFG VS 清零，所有例外和中断的入口地址是同一个。
	 * clear Vint cofigure
	 */
  li.d      $t0, (0x7 << 16)
  csrxchg   $zero, $t0, LOONGARCH_CSR_ECFG

	/* CSR EENTRY 异常入口 = 0x9000 0000 1c00 1000
	 * set ebase address
	 */
  li.d      $t0, PHYS_TO_CACHED(BOOT_SPACE_BASE + 0x1000)
  csrwr     $t0, LOONGARCH_CSR_EBASE

	/* CSR TLBRENTRY 异常入口 = 0x1c00 2000
	 * set TLB excption address
	 */
  li.d      $t0, BOOT_SPACE_BASE + 0x2000
  csrwr     $t0, LOONGARCH_CSR_TLBREBASE

  /* CPUCFG 0x3 LLEXC 为 1 表示支持 LL 指令取独占块功能
   * enable llexc
   */
  li.w      $t0, (1 << 3)
  csrxchg   $zero, $t0, LOONGARCH_CSR_MCSR1

  /* CRMD.IE 当前全局中断使能，高有效
   * disable interrupt
   */
  li.d      $t0, (1 << 2)
  csrxchg   $zero, $t0, LOONGARCH_CSR_CRMD

  /* lowlevel and clock init */
  bl        ls_lowlevel_init
  nop

  /* Lock L2 Cache as RAM */
  bl        ls_lock_scache
  nop

  /* 跳转到一致可缓存类型内存窗口继续执行 */
  li.d      $t1, CACHED_MEMORY_ADDR
  pcaddi    $t0, 3
  or        $t0, $t1, $t0
  jirl      $zero, $t0, 0
  /* now pc run to 0x90xxxxxxxxxxxxxx */

  /* DATF = 0b01 一致可缓存类型, DATM = 0b01 一致可缓存类型
   * PG = 1, DA = 0 使能映射地址翻译模式
   * DA disable for 0x80xxxxxxxxxxxxxx and 0x90xxxxxxxxxxxx address can be used
   */
  li.w      $t0, 0xb0
  csrwr     $t0, LOONGARCH_CSR_CRMD

  /* set idle stack */
  li.d      $t0, LS_IDLESTACK_TOP
  move      $sp, $t0

  /* Print "Hello LoongArch!!!" */
  EARLY_UART_INIT
  la        $a0, hello_str
  bl        early_print
  nop

  /* Jump to __ls_start */
  bl        __ls_start
  nop

  .size __start, . - __start

/****************************************************************************
 * Early print string
 ****************************************************************************/

  .section .text.early_print, "ax", @progbits
  .global early_print
  .type  early_print, function

early_print:
  move      $t4, $a0
1:
  ld.b      $t5, $t4, 0
  beqz      $t5, 2f
  li.d      $t2, EARLY_UART_LSR
3:
  ld.b      $t3, $t2, 0
  andi      $t3, $t3, EARLY_UART_LSR_THRE
  beqz      $t3, 3b
  li.d      $t2, EARLY_UART_THR
  st.b      $t5, $t2, 0
  addi.d    $t4, $t4, 1
  b         1b
2:
  ret

  .size early_print, . - early_print

hello_str:
  .ascii "\r\nHello, LoongArch!!!\r\n\0"

/****************************************************************************
 * Low-level initialization
 ****************************************************************************/

  .section .text.ls_lowlevel_init, "ax", @progbits
  .global ls_lowlevel_init
  .type  ls_lowlevel_init, function

ls_lowlevel_init:

  move      $a4, $ra

  /* CPUCFG 0x4 CC_FREQ: 恒定频率计时器/定时器晶振频率 0x0727 0E00 (120MHz)
   * CPUCFG 0x5 CC_MUL: 恒频倍频因子 = 0x1
   * CPUCFG 0x5 CC_DIV: 恒频分频系数 = 0x1
   */
  li.d      $t0, 0x0001000107270e00
  csrwr     $t0, LOONGARCH_CSR_MCSR2

  /* 芯片通用配置寄存器 00: ddr4_regs_default 窗口不命中处理
   * 1: 当访问内存控制器所有窗口不命中时，由内存配置空间默认给出响应，防止内存访问卡死
   */
  li.d      $t0, PHYS_TO_UNCACHED(LS_GENERAL_CFG0)
  ld.w      $t1, $t0, 0x0
  li.w      $t2, (1 << 5)
  or        $t1, $t1, $t2
  st.w      $t1, $t0, 0x0

  /* Enable SPI0 XIP fast read
   * SFC PARAM CLK_DIV=4, Enable fast_read
   */
  li.d      $t0, PHYS_TO_UNCACHED(LS_SPI0_BASE)
  li.w      $t1, 0x47
  st.b      $t1, $t0, 0x4

  /* close usb & ddr soft reset: [0] ddr, [7] usbm */
  li.w      $t1, 0xffffff7e
  st.w      $t1, $t0, 0x1c

  /* unvalid ddr soft reset: [0] ddr */
  ld.w      $t1, $t0, 0x1c
  ori       $t1, $t1, 0x1
  st.w      $t1, $t0, 0x1c

  /* Enable internal cache access */
  li.d      $t0, PHYS_TO_UNCACHED(LS_GENERAL_CFG3)
  ld.w      $t1, $t0, 0x0
  li.w      $t2, ((0x1f << 1) | (0x1f << 9))
  or        $t1, $t1, $t2
  st.w      $t1, $t0, 0x0

  /* GPIO40-41 MUX UART0 */
  li.d      $t0, PHYS_TO_UNCACHED(LS_GPIO_32_47_MULTI_CFG)
  li.w      $t2, 0x000f0000
  st.w      $t2, $t0, 0x0

  /* init PLL */
  bl        ls_clk_init
  nop

  move      $ra, $a4
  ret

  .size ls_lowlevel_init, . - ls_lowlevel_init

/****************************************************************************
 * Clock initialization (PLL configuration)
 *
 * REF_FREQ = 100MHz, CORE_FREQ = 1000MHz, DDR_FREQ = 800MHz
 * REF_FREQ = 120MHz, CORE_FREQ = 1000MHz, DDR_FREQ = 800MHz
 * APB_FREQ = 200MHz, NET_FREQ = 200MHz
 *
 * NODE_PLL: REFC=6, DIV=2, LOOPC=100
 *   GMAC_DIV=16, I2S_DIV=20
 * DDR_PLL: REFC=3, DIV=2, LOOPC=40
 *   DEV_DIV=8, NETWORK_DIV=8
 ****************************************************************************/

  .section .text.ls_clk_init, "ax", @progbits
  .global ls_clk_init
  .type  ls_clk_init, function

ls_clk_init:

#define NODE_L1DIV_OUT_SHIFT    24
#define NODE_L1DIV_LOOPC_SHIFT  15
#define NODE_L1DIV_REF_SHIFT    8
#define NODE_L2DIV_OUT_I2S_SHIFT 8
#define NODE_L2DIV_OUT_GMAC_SHIFT 0
#define NODE_L2DIV_OUT_I2S_MARK  0x7f
#define NODE_L2DIV_OUT_GMAC_MARK 0x7f

#define DDR_L1DIV_OUT_SHIFT     24
#define DDR_L1DIV_LOOPC_SHIFT   15
#define DDR_L1DIV_REF_SHIFT     8
#define DDR_L2DIV_OUT_MEMDIV_SHIFT 18
#define DDR_L2DIV_OUT_DEV_SHIFT  8
#define DDR_L2DIV_OUT_NET_SHIFT  0
#define DDR_L2DIV_OUT_MEMDIV_MARK 0x3
#define DDR_L2DIV_OUT_DEV_MARK   0x7f
#define DDR_L2DIV_OUT_NET_MARK   0x7f

#define PLL_L1_ENA      (0x1 << 3)
#define PLL_L1_PD_PLL   (0x1 << 5)
#define PLL_L1_LOCKED   (0x1 << 7)
#define SEL_PLL0        (0x1)
#define SEL_PLL1        (0x2)
#define SEL_PLL2        (0x4)

  li.d      $t0, LS_NODE_PLL_L
  li.w      $t1, PLL_L1_PD_PLL
  st.w      $t1, $t0, 0
  li.w      $t4, (2 << NODE_L1DIV_OUT_SHIFT) | (100 << NODE_L1DIV_LOOPC_SHIFT) | (6 << NODE_L1DIV_REF_SHIFT)
  st.w      $t4, $t0, 0

  ld.w      $t1, $t0, 4
  li.w      $t2, ~((NODE_L2DIV_OUT_I2S_MARK << NODE_L2DIV_OUT_I2S_SHIFT) | NODE_L2DIV_OUT_GMAC_MARK)
  and       $t1, $t1, $t2
  li.w      $t2, (20 << NODE_L2DIV_OUT_I2S_SHIFT) | 16
  or        $t2, $t2, $t1
  st.w      $t2, $t0, 0x4

  ori       $t4, $t4, PLL_L1_ENA
  st.w      $t4, $t0, 0

1:
  ld.w      $a0, $t0, 0
  li.w      $a1, PLL_L1_LOCKED
  and       $a0, $a1, $a0
  beqz      $a0, 1b

  ld.w      $a0, $t0, 0
  ori       $a0, $a0, SEL_PLL0 | SEL_PLL1 | SEL_PLL2
  st.w      $a0, $t0, 0

  li.d      $t0, LS_DDR_PLL_L
  ld.w      $t1, $t0, 4
  li.w      $t2, ~(DDR_L2DIV_OUT_MEMDIV_MARK << DDR_L2DIV_OUT_MEMDIV_SHIFT)
  and       $t1, $t1, $t2
  li.w      $t2, (1 << DDR_L2DIV_OUT_MEMDIV_SHIFT)
  or        $t2, $t2, $t1
  st.w      $t2, $t0, 0x4

  ld.w      $t1, $t0, 4
  li.w      $t2, ~(1 << 16)
  and       $t2, $t1, $t2
  st.w      $t2, $t0, 0x4
  li.w      $t2, (1 << 16)
  or        $t2, $t2, $t1
  st.w      $t2, $t0, 0x4

  li.w      $t1, PLL_L1_PD_PLL
  st.w      $t1, $t0, 0
  li.w      $t4, (2 << DDR_L1DIV_OUT_SHIFT) | (40 << DDR_L1DIV_LOOPC_SHIFT) | (3 << DDR_L1DIV_REF_SHIFT)
  st.w      $t4, $t0, 0

  ld.w      $t1, $t0, 4
  li.w      $t2, ~((DDR_L2DIV_OUT_DEV_MARK << DDR_L2DIV_OUT_DEV_SHIFT) | DDR_L2DIV_OUT_NET_MARK)
  and       $t1, $t1, $t2
  li.w      $t2, (8 << DDR_L2DIV_OUT_DEV_SHIFT) | 8
  or        $t2, $t2, $t1
  st.w      $t2, $t0, 0x4

  ori       $t4, $t4, PLL_L1_ENA
  st.w      $t4, $t0, 0

2:
  ld.w      $a0, $t0, 0
  li.w      $a1, PLL_L1_LOCKED
  and       $a0, $a0, $a1
  beqz      $a0, 2b

  ld.w      $a0, $t0, 0
  ori       $a0, $a0, SEL_PLL0 | SEL_PLL1 | SEL_PLL2
  st.w      $a0, $t0, 0

  li.w      $t1, 0x100
3:
  addi.d    $t1, $t1, -1
  bnez      $t1, 3b
  nop

  ret

  .size ls_clk_init, . - ls_clk_init

/****************************************************************************
 * Lock L2 cache as memory
 ****************************************************************************/

  .section .text.ls_lock_scache, "ax", @progbits
  .global ls_lock_scache
  .type  ls_lock_scache, function

ls_lock_scache:

  li.d      $t0, PHYS_TO_UNCACHED(LS_SCACHE_LOCK_WIN0_BASE)
  li.d      $t1, ~(LS_L2CACHE_SIZE - 1)
  REGSTORE  $t1, $t0, 0x40
  li.d      $t1, (LS_L2CACHE_BASE & 0xffffffffffff) | (1 << 63)
  REGSTORE  $t1, $t0, 0x0

  li.w      $t1, 0x100
1:
  addi.d    $t1, $t1, -1
  bnez      $t1, 1b
  nop

  ret

  .size ls_lock_scache, . - ls_lock_scache

/****************************************************************************
 * Exception vectors
 ****************************************************************************/

  .section .text.exception_vector, "ax", @progbits
  .global exception_vector
  .type  exception_vector, function

exception_vector:

  addi.d    $sp, $sp, -XCPTCONTEXT_SIZE

  save_ctx  $sp

  addi.d    $s0, $sp, XCPTCONTEXT_SIZE
  REGSTORE  $s0, $sp, REG_R3

  csrrd     $s0, LOONGARCH_CSR_ERA
  REGSTORE  $s0, $sp, REG_EPC

  csrrd     $s0, LOONGARCH_CSR_PRMD
  REGSTORE  $s0, $sp, REG_INT_CTX

  csrrd     $s1, LOONGARCH_CSR_ESTAT
  srli.d    $s2, $s1, CSR_ESTAT_EXC_SHIFT
  andi      $s2, $s2, 0x3f

  bnez      $s2, .Lexc_handle_exception

  li.d      $s3, 0x7fff
  and       $s3, $s1, $s3
  beqz      $s3, .Lexc_spurious_interrupt

  ctz.d     $s1, $s3
  addi.d    $s1, $s1, EXCCODE_INT_START
  b         .Lexc_has_irq

.Lexc_handle_exception:
  move      $a0, $s2
  move      $a1, $sp
  bl        loongarch_doirq
  b         .Lexc_return

.Lexc_spurious_interrupt:
  move      $a0, $sp
  b         .Lexc_return

.Lexc_has_irq:
  move      $a0, $s1
  move      $a1, $sp
  bl        loongarch_doirq
  b         .Lexc_return

.Lexc_return:
  move      $sp, $a0

  REGLOAD   $s0, $sp, REG_EPC
  csrwr     $s0, LOONGARCH_CSR_ERA

  REGLOAD   $s0, $sp, REG_INT_CTX
  csrwr     $s0, LOONGARCH_CSR_PRMD

  load_ctx  $sp

  REGLOAD   $sp, $sp, REG_R3

  ertn

  .size exception_vector, . - exception_vector

/****************************************************************************
 * TLB exception vector
 ****************************************************************************/

  .section .text.tlb_exception_vector, "ax", @progbits
  .global tlb_exception_vector
  .type  tlb_exception_vector, function

tlb_exception_vector:

  addi.d    $sp, $sp, -XCPTCONTEXT_SIZE

  save_ctx  $sp

  addi.d    $t1, $sp, XCPTCONTEXT_SIZE
  REGSTORE  $t1, $sp, REG_R3

  csrrd     $t1, LOONGARCH_CSR_TLBRERA
  REGSTORE  $t1, $sp, REG_EPC

  csrrd     $t1, LOONGARCH_CSR_PRMD
  REGSTORE  $t1, $sp, REG_INT_CTX

  li.w      $a0, LOONGARCH_IRQ_ASYNC
  move      $a1, $sp
  li.w      $a2, 0

  bl        loongarch_exception
  nop

  load_ctx  $sp

  REGLOAD   $t0, $sp, REG_EPC
  csrwr     $t0, LOONGARCH_CSR_TLBRERA

  REGLOAD   $t0, $sp, REG_INT_CTX
  csrwr     $t0, LOONGARCH_CSR_PRMD

  REGLOAD   $sp, $sp, REG_R3

  ertn

  .size tlb_exception_vector, . - tlb_exception_vector

  .end
