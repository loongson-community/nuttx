/****************************************************************************
 * arch/loongarch/src/ls2k/ls_oneshot.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_ONESHOT_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_ONESHOT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <time.h>

#include <nuttx/irq.h>

#include "ls_tim.h"

#ifdef CONFIG_LS_ONESHOT

#if !defined(CONFIG_LS_ONESHOT_MAXTIMERS) || \
    CONFIG_LS_ONESHOT_MAXTIMERS < 1
#  undef CONFIG_LS_ONESHOT_MAXTIMERS
#  define CONFIG_LS_ONESHOT_MAXTIMERS 1
#endif

#if CONFIG_LS_ONESHOT_MAXTIMERS > 8
#  warning Additional logic required to handle more than 8 timers
#  undef CONFIG_LS_ONESHOT_MAXTIMERS
#  define CONFIG_LS_ONESHOT_MAXTIMERS 8
#endif

typedef void (*oneshot_handler_t)(void *arg);

struct ls_oneshot_s
{
  uint8_t chan;
#if CONFIG_LS_ONESHOT_MAXTIMERS > 1
  uint8_t cbndx;
#endif
  volatile bool running;
  struct ls_tim_dev_s *tch;
  volatile oneshot_handler_t handler;
  volatile void *arg;
  uint32_t frequency;
  uint32_t period;
};

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

int ls_oneshot_initialize(struct ls_oneshot_s *oneshot, int chan,
                          uint16_t resolution);
int ls_oneshot_max_delay(struct ls_oneshot_s *oneshot, uint64_t *usec);
int ls_oneshot_start(struct ls_oneshot_s *oneshot,
                     oneshot_handler_t handler, void *arg,
                     const struct timespec *ts);
int ls_oneshot_cancel(struct ls_oneshot_s *oneshot,
                      struct timespec *ts);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* CONFIG_LS_ONESHOT */

#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_ONESHOT_H */
