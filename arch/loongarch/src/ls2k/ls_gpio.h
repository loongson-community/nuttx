/****************************************************************************
 * arch/loongarch/src/ls2k/ls_gpio.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_GPIO_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_GPIO_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <stdint.h>
#  include <stdbool.h>
#endif

#include "chip.h"

#include "hardware/ls_gpio.h"

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

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

/****************************************************************************
 * Name: ls_configgpio
 *
 * Description:
 *   Configure a GPIO pin based on bit-encoded description of the pin.
 *
 * Returned Value:
 *   OK on success
 *   ERROR on error(?)
 *
 ****************************************************************************/

int ls_configgpio(uint32_t cfgset);

/****************************************************************************
 * Name: ls_unconfiggpio
 *
 * Description:
 *   Unconfigure a GPIO pin, resetting it to the default state (GPIO
 *   function, input direction).
 *
 * Returned Value:
 *   OK on success
 *   ERROR on error(?)
 *
 ****************************************************************************/

int ls_unconfiggpio(uint32_t cfgset);

/****************************************************************************
 * Name: ls_gpiowrite
 *
 * Description:
 *   Write one or zero to the selected GPIO pin
 *
 ****************************************************************************/

void ls_gpiowrite(uint32_t pinset, bool value);

/****************************************************************************
 * Name: ls_gpioread
 *
 * Description:
 *   Read one or zero from the selected GPIO pin
 *
 ****************************************************************************/

bool ls_gpioread(uint32_t pinset);

/****************************************************************************
 * Name: ls_gpioinit
 *
 * Description:
 *   Initialize GPIO. Called early in the boot sequence.
 *
 *   Typically called from ls_start().
 *
 ****************************************************************************/

void ls_gpioinit(void);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_GPIO_H */
