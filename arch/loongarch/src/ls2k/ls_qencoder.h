/****************************************************************************
 * arch/loongarch/src/ls2k/ls_qencoder.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_QENCODER_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_QENCODER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "chip.h"

#ifdef CONFIG_SENSORS_QENCODER

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Timer devices may be used for different purposes.  One special purpose is
 * as a quadrature encoder input device.  If CONFIG_LS_TIMn is defined
 * then CONFIG_LS_TIMn_QE indicates that timer "n" is intended to be used
 * as a quadrature encoder.
 */

#ifndef CONFIG_LS_TIM1
#  undef CONFIG_LS_TIM1_QE
#endif
#ifndef CONFIG_LS_TIM2
#  undef CONFIG_LS_TIM2_QE
#endif

/* Basic and small general-purpose timers do not support encoder mode. */

#undef CONFIG_LS_TIM6_QE

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: ls_qeinitialize
 *
 * Description:
 *   Initialize a quadrature encoder interface.  This function must be called
 *   from board-specific logic.
 *
 * Input Parameters:
 *   devpath - The full path to the driver to register. E.g., "/dev/qe0"
 *   tim     - The timer number to use.
 *
 * Returned Value:
 *   Zero on success; A negated errno value is returned on failure.
 *
 ****************************************************************************/

int ls_qeinitialize(const char *devpath, int tim);

#endif /* CONFIG_SENSORS_QENCODER */

#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_QENCODER_H */
