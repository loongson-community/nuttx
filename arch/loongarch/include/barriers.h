/****************************************************************************
 * arch/loongarch/include/barriers.h
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

#ifndef __ARCH_LOONGARCH_INCLUDE_BARRIERS_H
#define __ARCH_LOONGARCH_INCLUDE_BARRIERS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* See LoongArch Reference Manual Vol 1 V1.11
 * 2.2.8.1
 */

#define __DBAR(hint) __asm__ __volatile__ ("dbar " #hint ::: "memory")

#define UP_DMB()  __DBAR(0b10000)
#define UP_RMB()  __DBAR(0b10101)
#define UP_WMB()  __DBAR(0b11010)
#define UP_DSB()  __DBAR(0b00000)

/* See LoongArch Reference Manual Vol 1 V1.11
 * 2.2.8.2
 */

#define UP_ISB()  __asm__ __volatile__ ("ibar 0" ::: "memory")

#endif /* __ARCH_LOONGARCH_INCLUDE_BARRIERS_H */
