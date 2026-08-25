/****************************************************************************
 * arch/loongarch/src/ls2k/ls_tim.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_TIM_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_TIM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "chip.h"
#include "hardware/ls_tim.h"

#include <nuttx/irq.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Helpers ******************************************************************/

#define LS_TIM_SETMODE(d,mode)        ((d)->ops->setmode(d,mode))
#define LS_TIM_SETCLOCK(d,freq)       ((d)->ops->setclock(d,freq))
#define LS_TIM_SETPERIOD(d,period)    ((d)->ops->setperiod(d,period))
#define LS_TIM_GETCOUNTER(d)          ((d)->ops->getcounter(d))
#define LS_TIM_SETCOUNTER(d,c)        ((d)->ops->setcounter(d,c))
#define LS_TIM_GETWIDTH(d)            ((d)->ops->getwidth(d))
#define LS_TIM_SETCHANNEL(d,ch,mode)  ((d)->ops->setchannel(d,ch,mode))
#define LS_TIM_SETCOMPARE(d,ch,comp)  ((d)->ops->setcompare(d,ch,comp))
#define LS_TIM_GETCAPTURE(d,ch)       ((d)->ops->getcapture(d,ch))
#define LS_TIM_SETISR(d,hnd,arg,s)    ((d)->ops->setisr(d,hnd,arg,s))
#define LS_TIM_ENABLEINT(d,s)         ((d)->ops->enableint(d,s))
#define LS_TIM_DISABLEINT(d,s)        ((d)->ops->disableint(d,s))
#define LS_TIM_ACKINT(d,s)            ((d)->ops->ackint(d,s))
#define LS_TIM_CHECKINT(d,s)          ((d)->ops->checkint(d,s))
#define LS_TIM_ENABLE(d)              ((d)->ops->enable(d))
#define LS_TIM_DISABLE(d)             ((d)->ops->disable(d))

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

/* TIM Device Structure */

struct ls_tim_dev_s
{
  struct ls_tim_ops_s *ops;
};

/* TIM Modes of Operation */

typedef enum
{
  LS_TIM_MODE_UNUSED        = -1,

  /* One of the following */

  LS_TIM_MODE_MASK          = 0x0310,
  LS_TIM_MODE_DISABLED      = 0x0000,
  LS_TIM_MODE_UP            = 0x0100,
  LS_TIM_MODE_DOWN          = 0x0110,
  LS_TIM_MODE_UPDOWN        = 0x0200,
  LS_TIM_MODE_PULSE         = 0x0300,

  /* One of the following */

  LS_TIM_MODE_CK_INT        = 0x0000,

  /* LS_TIM_MODE_CK_INT_TRIG   = 0x0400, */

  /* LS_TIM_MODE_CK_EXT        = 0x0800, */

  /* LS_TIM_MODE_CK_EXT_TRIG   = 0x0C00, */

  /* Clock sources, OR'ed with CK_EXT */

  /* LS_TIM_MODE_CK_CHINVALID  = 0x0000, */

  /* LS_TIM_MODE_CK_CH1        = 0x0001, */

  /* LS_TIM_MODE_CK_CH2        = 0x0002, */

  /* LS_TIM_MODE_CK_CH3        = 0x0003, */

  /* LS_TIM_MODE_CK_CH4        = 0x0004  */

  /* Todo: external trigger block */
} ls_tim_mode_t;

/* TIM Channel Modes */

typedef enum
{
  LS_TIM_CH_DISABLED        = 0x00,

  /* Common configuration */

  LS_TIM_CH_POLARITY_POS    = 0x00,
  LS_TIM_CH_POLARITY_NEG    = 0x01,

  /* MODES: */

  LS_TIM_CH_MODE_MASK       = 0x06,

  /* Output Compare Modes */

  LS_TIM_CH_OUTPWM          = 0x04,     /* Enable standard PWM mode, active high when counter < compare */

  /* LS_TIM_CH_OUTCOMPARE      = 0x06, */

  /* TODO other modes ... as PWM capture, ENCODER and Hall Sensor */

  /* LS_TIM_CH_INCAPTURE       = 0x10, */

  /* LS_TIM_CH_INPWM           = 0x20  */

  /* LS_TIM_CH_DRIVE_OC   -- open collector mode */
} ls_tim_channel_t;

/* TIM Operations */

struct ls_tim_ops_s
{
  /* Basic Timers */

  void (*enable)(struct ls_tim_dev_s *dev);
  void (*disable)(struct ls_tim_dev_s *dev);
  int  (*setmode)(struct ls_tim_dev_s *dev, ls_tim_mode_t mode);
  int  (*setclock)(struct ls_tim_dev_s *dev, uint32_t freq);
  void (*setperiod)(struct ls_tim_dev_s *dev, uint32_t period);
  uint32_t (*getcounter)(struct ls_tim_dev_s *dev);
  void (*setcounter)(struct ls_tim_dev_s *dev, uint32_t count);

  /* General and Advanced Timers Adds */

  int  (*getwidth)(struct ls_tim_dev_s *dev);
  int  (*setchannel)(struct ls_tim_dev_s *dev, uint8_t channel,
                     ls_tim_channel_t mode);
  int  (*setcompare)(struct ls_tim_dev_s *dev, uint8_t channel,
                     uint32_t compare);
  int  (*getcapture)(struct ls_tim_dev_s *dev, uint8_t channel);

  /* Timer interrupts */

  int  (*setisr)(struct ls_tim_dev_s *dev,
                 xcpt_t handler, void * arg, int source);
  void (*enableint)(struct ls_tim_dev_s *dev, int source);
  void (*disableint)(struct ls_tim_dev_s *dev, int source);
  void (*ackint)(struct ls_tim_dev_s *dev, int source);
  int  (*checkint)(struct ls_tim_dev_s *dev, int source);
};

/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/

/* Power-up timer and get its structure */

struct ls_tim_dev_s *ls_tim_init(int timer);

/* Power-down timer, mark it as unused */

int ls_tim_deinit(struct ls_tim_dev_s *dev);

/****************************************************************************
 * Name: ls_timer_initialize
 *
 * Description:
 *   Bind the configuration timer to a timer lower half instance and
 *   register the timer drivers at 'devpath'
 *
 * Input Parameters:
 *   devpath - The full path to the timer device.
 *              This should be of the form /dev/timer0
 *   timer - the timer number.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returned
 *   to indicate the nature of any failure.
 *
 ****************************************************************************/

#ifdef CONFIG_TIMER
int ls_timer_initialize(const char *devpath, int timer);
#endif

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_TIM_H */
