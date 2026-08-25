/****************************************************************************
 * arch/loongarch/src/common/loongarch_macros.S
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

.file "loongarch_macros.S"

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <arch/arch.h>
#include <arch/csr.h>
#include <arch/irq.h>

#include <sys/types.h>

#include "loongarch_internal.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Name: save_ctx
 *
 * Parameter:
 *   in - Pointer to where the save is performed (e.g. sp)
 *
 * Description:
 *   Save the common context registers (i.e. work / temp / etc).
 *
 ****************************************************************************/

.macro save_ctx in

  REGSTORE    $ra,  \in, REG_R1   /* ra */
  REGSTORE    $r2,  \in, REG_R2   /* tp */
  REGSTORE    $r4,  \in, REG_R4   /* a0 */
  REGSTORE    $r5,  \in, REG_R5   /* a1 */
  REGSTORE    $r6,  \in, REG_R6   /* a2 */
  REGSTORE    $r7,  \in, REG_R7   /* a3 */
  REGSTORE    $r8,  \in, REG_R8   /* a4 */
  REGSTORE    $r9,  \in, REG_R9   /* a5 */
  REGSTORE    $r10, \in, REG_R10  /* a6 */
  REGSTORE    $r11, \in, REG_R11  /* a7 */
  REGSTORE    $r12, \in, REG_R12  /* t0 */
  REGSTORE    $r13, \in, REG_R13  /* t1 */
  REGSTORE    $r14, \in, REG_R14  /* t2 */
  REGSTORE    $r15, \in, REG_R15  /* t3 */
  REGSTORE    $r16, \in, REG_R16  /* t4 */
  REGSTORE    $r17, \in, REG_R17  /* t5 */
  REGSTORE    $r18, \in, REG_R18  /* t6 */
  REGSTORE    $r19, \in, REG_R19  /* t7 */
  REGSTORE    $r20, \in, REG_R20  /* t8 */
  REGSTORE    $r21, \in, REG_R21  /* Reserved */
  REGSTORE    $r22, \in, REG_R22  /* fp */
  REGSTORE    $r23, \in, REG_R23  /* s0 */
  REGSTORE    $r24, \in, REG_R24  /* s1 */
  REGSTORE    $r25, \in, REG_R25  /* s2 */
  REGSTORE    $r26, \in, REG_R26  /* s3 */
  REGSTORE    $r27, \in, REG_R27  /* s4 */
  REGSTORE    $r28, \in, REG_R28  /* s5 */
  REGSTORE    $r29, \in, REG_R29  /* s6 */
  REGSTORE    $r30, \in, REG_R30  /* s7 */
  REGSTORE    $r31, \in, REG_R31  /* s8 */

.endm

/****************************************************************************
 * Name: loongarch_savefpu
 *
 * Parameter:
 *   in - Pointer to where the save is performed (e.g. sp)
 *
 * Description:
 *   Save the FPU context registers (i.e. work / temp / etc).
 *
 ****************************************************************************/

.macro loongarch_savefpu in

  /* Store all floating point registers */

  FSTORE      $f0,  \in, REG_F0
  FSTORE      $f1,  \in, REG_F1
  FSTORE      $f2,  \in, REG_F2
  FSTORE      $f3,  \in, REG_F3
  FSTORE      $f4,  \in, REG_F4
  FSTORE      $f5,  \in, REG_F5
  FSTORE      $f6,  \in, REG_F6
  FSTORE      $f7,  \in, REG_F7
  FSTORE      $f8,  \in, REG_F8
  FSTORE      $f9,  \in, REG_F9
  FSTORE      $f10, \in, REG_F10
  FSTORE      $f11, \in, REG_F11
  FSTORE      $f12, \in, REG_F12
  FSTORE      $f13, \in, REG_F13
  FSTORE      $f14, \in, REG_F14
  FSTORE      $f15, \in, REG_F15
  FSTORE      $f16, \in, REG_F16
  FSTORE      $f17, \in, REG_F17
  FSTORE      $f18, \in, REG_F18
  FSTORE      $f19, \in, REG_F19
  FSTORE      $f20, \in, REG_F20
  FSTORE      $f21, \in, REG_F21
  FSTORE      $f22, \in, REG_F22
  FSTORE      $f23, \in, REG_F23
  FSTORE      $f24, \in, REG_F24
  FSTORE      $f25, \in, REG_F25
  FSTORE      $f26, \in, REG_F26
  FSTORE      $f27, \in, REG_F27
  FSTORE      $f28, \in, REG_F28
  FSTORE      $f29, \in, REG_F29
  FSTORE      $f30, \in, REG_F30
  FSTORE      $f31, \in, REG_F31

  /* Store the floating point control and status register */

  movfcsr2gr  $t0, $fcsr0
  REGSTORE    $t0, \in, REG_FCSR

  /* Store the floating point condition codes ($fcc0..$fcc7). Each fcc is a
   * single bit; pack them into one GPR (fccN -> bit 8*N) and store it.
   */

  movcf2gr    $t0, $fcc0
  move        $t1, $t0
  movcf2gr    $t0, $fcc1
  bstrins.d   $t1, $t0, 15, 8
  movcf2gr    $t0, $fcc2
  bstrins.d   $t1, $t0, 23, 16
  movcf2gr    $t0, $fcc3
  bstrins.d   $t1, $t0, 31, 24
  movcf2gr    $t0, $fcc4
  bstrins.d   $t1, $t0, 39, 32
  movcf2gr    $t0, $fcc5
  bstrins.d   $t1, $t0, 47, 40
  movcf2gr    $t0, $fcc6
  bstrins.d   $t1, $t0, 55, 48
  movcf2gr    $t0, $fcc7
  bstrins.d   $t1, $t0, 63, 56
  REGSTORE    $t1, \in, REG_FCC

.endm

/****************************************************************************
 * Name: load_ctx
 *
 * Parameter:
 *   out - Pointer to where the load is performed (e.g. sp)
 *
 * Description:
 *   Load the common context registers (i.e. work / temp / etc).
 *
 ****************************************************************************/

.macro load_ctx out

  REGLOAD     $ra,  \out, REG_R1  /* ra */
  REGLOAD     $r2,  \out, REG_R2  /* tp */
  REGLOAD     $r4,  \out, REG_R4  /* a0 */
  REGLOAD     $r5,  \out, REG_R5  /* a1 */
  REGLOAD     $r6,  \out, REG_R6  /* a2 */
  REGLOAD     $r7,  \out, REG_R7  /* a3 */
  REGLOAD     $r8,  \out, REG_R8  /* a4 */
  REGLOAD     $r9,  \out, REG_R9  /* a5 */
  REGLOAD     $r10, \out, REG_R10 /* a6 */
  REGLOAD     $r11, \out, REG_R11 /* a7 */
  REGLOAD     $r12, \out, REG_R12 /* t0 */
  REGLOAD     $r13, \out, REG_R13 /* t1 */
  REGLOAD     $r14, \out, REG_R14 /* t2 */
  REGLOAD     $r15, \out, REG_R15 /* t3 */
  REGLOAD     $r16, \out, REG_R16 /* t4 */
  REGLOAD     $r17, \out, REG_R17 /* t5 */
  REGLOAD     $r18, \out, REG_R18 /* t6 */
  REGLOAD     $r19, \out, REG_R19 /* t7 */
  REGLOAD     $r20, \out, REG_R20 /* t8 */
  REGLOAD     $r21, \out, REG_R21 /* Reserved */
  REGLOAD     $r22, \out, REG_R22 /* fp */
  REGLOAD     $r23, \out, REG_R23 /* s0 */
  REGLOAD     $r24, \out, REG_R24 /* s1 */
  REGLOAD     $r25, \out, REG_R25 /* s2 */
  REGLOAD     $r26, \out, REG_R26 /* s3 */
  REGLOAD     $r27, \out, REG_R27 /* s4 */
  REGLOAD     $r28, \out, REG_R28 /* s5 */
  REGLOAD     $r29, \out, REG_R29 /* s6 */
  REGLOAD     $r30, \out, REG_R30 /* s7 */
  REGLOAD     $r31, \out, REG_R31 /* s8 */

.endm

/****************************************************************************
 * Name: loongarch_loadfpu
 *
 * Parameter:
 *   out - Pointer to where the load is performed (e.g. sp)
 *
 * Description:
 *   Load the FPU context registers (i.e. work / temp / etc).
 *
 ****************************************************************************/

.macro loongarch_loadfpu out

  /* Load all floating point registers */

  FLOAD       $f0,  \out, REG_F0
  FLOAD       $f1,  \out, REG_F1
  FLOAD       $f2,  \out, REG_F2
  FLOAD       $f3,  \out, REG_F3
  FLOAD       $f4,  \out, REG_F4
  FLOAD       $f5,  \out, REG_F5
  FLOAD       $f6,  \out, REG_F6
  FLOAD       $f7,  \out, REG_F7
  FLOAD       $f8,  \out, REG_F8
  FLOAD       $f9,  \out, REG_F9
  FLOAD       $f10, \out, REG_F10
  FLOAD       $f11, \out, REG_F11
  FLOAD       $f12, \out, REG_F12
  FLOAD       $f13, \out, REG_F13
  FLOAD       $f14, \out, REG_F14
  FLOAD       $f15, \out, REG_F15
  FLOAD       $f16, \out, REG_F16
  FLOAD       $f17, \out, REG_F17
  FLOAD       $f18, \out, REG_F18
  FLOAD       $f19, \out, REG_F19
  FLOAD       $f20, \out, REG_F20
  FLOAD       $f21, \out, REG_F21
  FLOAD       $f22, \out, REG_F22
  FLOAD       $f23, \out, REG_F23
  FLOAD       $f24, \out, REG_F24
  FLOAD       $f25, \out, REG_F25
  FLOAD       $f26, \out, REG_F26
  FLOAD       $f27, \out, REG_F27
  FLOAD       $f28, \out, REG_F28
  FLOAD       $f29, \out, REG_F29
  FLOAD       $f30, \out, REG_F30
  FLOAD       $f31, \out, REG_F31

  /* Load the floating point control and status register */

  REGLOAD     $t0,  \out, REG_FCSR
  movgr2fcsr  $fcsr0, $t0

  /* Load the floating point condition codes ($fcc0..$fcc7), unpacking the
   * packed value (fccN at bit 8*N) back into the individual fcc registers.
   */

  REGLOAD     $t0,  \out, REG_FCC
  bstrpick.d  $t1,  $t0,  7, 0
  movgr2cf    $fcc0, $t1
  bstrpick.d  $t1,  $t0, 15, 8
  movgr2cf    $fcc1, $t1
  bstrpick.d  $t1,  $t0, 23, 16
  movgr2cf    $fcc2, $t1
  bstrpick.d  $t1,  $t0, 31, 24
  movgr2cf    $fcc3, $t1
  bstrpick.d  $t1,  $t0, 39, 32
  movgr2cf    $fcc4, $t1
  bstrpick.d  $t1,  $t0, 47, 40
  movgr2cf    $fcc5, $t1
  bstrpick.d  $t1,  $t0, 55, 48
  movgr2cf    $fcc6, $t1
  bstrpick.d  $t1,  $t0, 63, 56
  movgr2cf    $fcc7, $t1

.endm
