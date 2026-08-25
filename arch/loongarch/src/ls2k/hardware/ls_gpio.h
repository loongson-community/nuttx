/****************************************************************************
 * arch/loongarch/src/ls2k/hardware/ls_gpio.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_GPIO_H
#define __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_GPIO_H

#define LS_GPIO_DIR_OFFSET          0x800
#define LS_GPIO_OUT_OFFSET          0x900
#define LS_GPIO_IN_OFFSET           0xa00
#define LS_GPIO_IRQ_OFFSET          0xb00
#define LS_GPIO_IRQPOL_OFFSET       0xc00
#define LS_GPIO_IRQEDG_OFFSET       0xd00
#define LS_GPIO_IRQCLR_OFFSET       0xe00
#define LS_GPIO_IRQSTA_OFFSET       0xf00
#define LS_GPIO_IRQDUL_OFFSET       0xf80

#define LS_GPIO_NPINS               106

#endif /* __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_GPIO_H */
