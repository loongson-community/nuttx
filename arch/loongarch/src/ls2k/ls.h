/****************************************************************************
 * arch/loongarch/src/ls2k/ls.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include "loongarch_internal.h"

/* Peripherals **************************************************************/

#include "chip.h"
#include "ls_lowputc.h"

#ifdef CONFIG_LS_DMA
#  include "ls_dma.h"
#endif

#ifdef CONFIG_LS_GPIO
#  include "ls_gpio.h"
#endif

#ifdef CONFIG_LS_I2C
#  include "ls_i2c.h"
#endif

#ifdef CONFIG_LS_QE
#  include "ls_qencoder.h"
#endif

#ifdef CONFIG_LS_TIM
#  include "ls_tim.h"
#endif

#ifdef CONFIG_LS_TIM_PWM
#  include "ls_tim_pwm.h"
#endif

#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_H */
