/****************************************************************************
 * arch/loongarch/src/ls2k/ls_capture.h
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

#ifndef __ARCH_ARM_SRC_COMMON_LS_LS_CAPTURE_H
#define __ARCH_ARM_SRC_COMMON_LS_LS_CAPTURE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <stdint.h>

#include <nuttx/irq.h>
#include <nuttx/timers/capture.h>

#include "chip.h"
#include <arch/board/board.h>
#include "hardware/ls_tim.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Helpers ******************************************************************/

#define LS_CAP_SETSMC(d,cfg)                 ((d)->ops->setsmc(d,cfg))
#define LS_CAP_SETCLOCK(d,clk,max)           ((d)->ops->setclock(d,clk,max))
#define LS_CAP_SETCHANNEL(d,ch,cfg)          ((d)->ops->setchannel(d,ch,cfg))
#define LS_CAP_GETCAPTURE(d,ch)              ((d)->ops->getcapture(d,ch))
#define LS_CAP_SETISR(d,hnd,arg)             ((d)->ops->setisr(d,hnd,arg))
#define LS_CAP_ENABLEINT(d,s,on)             ((d)->ops->enableint(d,s,on))
#define LS_CAP_ACKFLAGS(d,f)                 ((d)->ops->ackflags(d,f))
#define LS_CAP_GETFLAGS(d)                   ((d)->ops->getflags(d))
#define LS_CAP_RSTCOUNTER(d)                 ((d)->ops->rstcounter(d))

/****************************************************************************
 * Public Types
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

struct ls_cap_dev_s
{
  struct ls_cap_ops_s *ops;
};

typedef enum
{
  LS_CAP_MAPPED_MASK         = (GTIM_CCMR1_CC1S_MASK),
  LS_CAP_MAPPED_TI1          = (GTIM_CCMR_CCS_CCIN1),
  LS_CAP_MAPPED_TI2          = (GTIM_CCMR_CCS_CCIN2),
  LS_CAP_MAPPED_TI3          = (GTIM_CCMR_CCS_CCIN1),
  LS_CAP_MAPPED_TI4          = (GTIM_CCMR_CCS_CCIN2),
  LS_CAP_INPSC_MASK          = (GTIM_CCMR1_IC1PSC_MASK),
  LS_CAP_INPSC_NO            = (0 << GTIM_CCMR1_IC1PSC_SHIFT),
  LS_CAP_INPSC_2EVENTS       = (1 << GTIM_CCMR1_IC1PSC_SHIFT),
  LS_CAP_INPSC_4EVENTS       = (2 << GTIM_CCMR1_IC1PSC_SHIFT),
  LS_CAP_INPSC_8EVENTS       = (3 << GTIM_CCMR1_IC1PSC_SHIFT),
  LS_CAP_FILTER_MASK         = (GTIM_CCMR1_IC1F_MASK),
  LS_CAP_FILTER_NO           = (0 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_INT_N2       = (1 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_INT_N4       = (2 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_INT_N8       = (3 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D2_N6    = (4 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D2_N8    = (5 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D4_N6    = (6 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D4_N8    = (7 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D8_N6    = (8 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D8_N8    = (9 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D16_N5   = (10 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D16_N6   = (11 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D16_N8   = (12 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D32_N5   = (13 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D32_N6   = (14 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_FILTER_DTS_D32_N8   = (15 << GTIM_CCMR1_IC1F_SHIFT),
  LS_CAP_EDGE_MASK           = (3 << 8),
  LS_CAP_EDGE_DISABLED       = (0 << 8),
  LS_CAP_EDGE_RISING         = (1 << 8),
  LS_CAP_EDGE_FALLING        = (2 << 8),
  LS_CAP_EDGE_BOTH           = (3 << 8),
} ls_cap_ch_cfg_t;

typedef enum
{
  LS_CAP_SMS_MASK            = (7 << GTIM_SMCR_SMS_SHIFT),
  LS_CAP_SMS_INT             = (0 << GTIM_SMCR_SMS_SHIFT),
  LS_CAP_SMS_ENC1            = (1 << GTIM_SMCR_SMS_SHIFT),
  LS_CAP_SMS_ENC2            = (2 << GTIM_SMCR_SMS_SHIFT),
  LS_CAP_SMS_ENC3            = (3 << GTIM_SMCR_SMS_SHIFT),
  LS_CAP_SMS_RST             = (4 << GTIM_SMCR_SMS_SHIFT),
  LS_CAP_SMS_GAT             = (5 << GTIM_SMCR_SMS_SHIFT),
  LS_CAP_SMS_TRG             = (6 << GTIM_SMCR_SMS_SHIFT),
  LS_CAP_SMS_EXT             = (7 << GTIM_SMCR_SMS_SHIFT),
  LS_CAP_TS_MASK             = (7 << GTIM_SMCR_TS_SHIFT),
  LS_CAP_TS_ITR0             = (0 << GTIM_SMCR_TS_SHIFT),
  LS_CAP_TS_ITR1             = (1 << GTIM_SMCR_TS_SHIFT),
  LS_CAP_TS_ITR2             = (2 << GTIM_SMCR_TS_SHIFT),
  LS_CAP_TS_ITR3             = (3 << GTIM_SMCR_TS_SHIFT),
  LS_CAP_TS_TI1FED           = (4 << GTIM_SMCR_TS_SHIFT),
  LS_CAP_TS_TI1FP1           = (5 << GTIM_SMCR_TS_SHIFT),
  LS_CAP_TS_TI2FP2           = (6 << GTIM_SMCR_TS_SHIFT),
  LS_CAP_TS_ETRF             = (7 << GTIM_SMCR_TS_SHIFT),
  LS_CAP_MSM_MASK            = (1 << 7)
} ls_cap_smc_cfg_t;

typedef enum
{
  LS_CAP_FLAG_IRQ_COUNTER    = (GTIM_SR_UIF),
  LS_CAP_FLAG_IRQ_CH_1       = (GTIM_SR_CC1IF),
  LS_CAP_FLAG_IRQ_CH_2       = (GTIM_SR_CC2IF),
  LS_CAP_FLAG_IRQ_CH_3       = (GTIM_SR_CC3IF),
  LS_CAP_FLAG_IRQ_CH_4       = (GTIM_SR_CC4IF),
  LS_CAP_FLAG_OF_CH_1        = (GTIM_SR_CC1OF),
  LS_CAP_FLAG_OF_CH_2        = (GTIM_SR_CC2OF),
  LS_CAP_FLAG_OF_CH_3        = (GTIM_SR_CC3OF),
  LS_CAP_FLAG_OF_CH_4        = (GTIM_SR_CC4OF)
} ls_cap_flags_t;

#define LS_CAP_FLAG_IRQ_CH(ch)   (GTIM_SR_CC1IF<<((ch)-1))
#define LS_CAP_FLAG_OF_CH(ch)    (GTIM_SR_CC1OF<<((ch)-1))
#define LS_CAP_CHANNEL_COUNTER   0

struct ls_cap_ops_s
{
  int  (*setsmc)(struct ls_cap_dev_s *dev, ls_cap_smc_cfg_t cfg);
  int  (*setclock)(struct ls_cap_dev_s *dev, uint32_t freq,
                   uint32_t max);
  int  (*setchannel)(struct ls_cap_dev_s *dev, uint8_t channel,
                     ls_cap_ch_cfg_t cfg);
  uint32_t (*getcapture)(struct ls_cap_dev_s *dev, uint8_t channel);
  int  (*setisr)(struct ls_cap_dev_s *dev, xcpt_t handler, void *arg);
  void (*enableint)(struct ls_cap_dev_s *dev, ls_cap_flags_t src,
                    bool on);
  void (*ackflags)(struct ls_cap_dev_s *dev, int flags);
  ls_cap_flags_t (*getflags)(struct ls_cap_dev_s *dev);
  uint32_t (*rstcounter)(struct ls_cap_dev_s *dev);
};

struct ls_cap_dev_s *ls_cap_init(int timer);
int ls_cap_deinit(struct ls_cap_dev_s *dev);
struct cap_lowerhalf_s *ls_cap_initialize(int timer);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */

#endif /* __ARCH_ARM_SRC_COMMON_LS_LS_CAPTURE_H */
