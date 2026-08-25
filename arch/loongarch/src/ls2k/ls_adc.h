/****************************************************************************
 * arch/loongarch/src/ls2k/ls_adc.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_ADC_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_ADC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "chip.h"

#include "hardware/ls_adc.h"

#include <nuttx/analog/adc.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Generalized definitions for ADC  *****************************************/

#define LS_ADC_DMAREG_OFFSET         LS_ADC_CR2_OFFSET
#define ADC_DMAREG_DMA               ADC_CR2_DMA
#define LS_ADC_EXTREG_OFFSET         LS_ADC_CR2_OFFSET
#define ADC_EXTREG_EXTSEL_MASK       ADC_CR2_EXTSEL_MASK
#define ADC_EXTREG_EXTSEL_SHIFT      ADC_CR2_EXTSEL_SHIFT
#define LS_ADC_JEXTREG_OFFSET        LS_ADC_CR2_OFFSET
#define ADC_JEXTREG_JEXTSEL_MASK     ADC_CR2_JEXTSEL_MASK
#define ADC_EXTREG_JEXTSEL_SHIFT     ADC_CR2_JEXTSEL_SHIFT
#define LS_ADC_ISR_OFFSET            LS_ADC_SR_OFFSET
#define LS_ADC_IER_OFFSET            LS_ADC_CR1_OFFSET
#define ADC_EXTREG_EXTEN_MASK        ADC_CR2_EXTTRIG
#define ADC_EXTREG_EXTEN_NONE        0
#define ADC_EXTREG_EXTEN_DEFAULT     ADC_CR2_EXTTRIG
#define ADC_JEXTREG_JEXTEN_MASK      ADC_CR2_JEXTTRIG
#define ADC_JEXTREG_JEXTEN_NONE      0
#define ADC_JEXTREG_JEXTEN_DEFAULT   ADC_CR2_JEXTTRIG

/* Configuration ************************************************************/

/* Timer devices may be used for different purposes.  One special purpose is
 * to control periodic ADC sampling.  If CONFIG_LS_TIMn is defined then
 * CONFIG_LS_TIMn_ADC must also be defined to indicate that timer "n" is
 * intended to be used for that purpose.
 */

/* For the LS2K0300, timers 1, 2, 6 may be used. */

#ifndef CONFIG_LS_TIM1
#  undef CONFIG_LS_TIM1_ADC
#  undef CONFIG_LS_TIM1_ADC1
#endif

#ifndef CONFIG_LS_TIM2
#  undef CONFIG_LS_TIM2_ADC
#  undef CONFIG_LS_TIM2_ADC1
#endif

#ifndef CONFIG_LS_TIM6
#  undef CONFIG_LS_TIM6_ADC
#  undef CONFIG_LS_TIM6_ADC1
#endif

/* LS2K0300 provides one ADC interface. */

#ifdef CONFIG_LS_ADC1

/* DMA support */

#undef ADC_HAVE_DMA
#ifdef CONFIG_LS_ADC1_DMA
#  define ADC_HAVE_DMA  1
#endif

#ifdef CONFIG_LS_ADC1_DMA
#  define ADC1_HAVE_DMA 1
#else
#  undef  ADC1_HAVE_DMA
#endif

/* Injected channels support */

#if CONFIG_LS_ADC1_INJECTED_CHAN > 0
#  define ADC_HAVE_INJECTED
#endif

/* Timer configuration: If a timer trigger is specified, then get
 * information about the timer.  LS2K0300 has one ADC and TIM1, TIM2 and
 * TIM6 can be selected as trigger generators.
 */

#if defined(CONFIG_LS_TIM1_ADC1)
#    define ADC1_HAVE_TIMER           1
#    define ADC1_TIMER_BASE           LS_TIM1_BASE
#    define ADC1_TIMER_PCLK_FREQUENCY LS_TIM1_CLKIN
#elif defined(CONFIG_LS_TIM2_ADC1)
#    define ADC1_HAVE_TIMER           1
#    define ADC1_TIMER_BASE           LS_TIM2_BASE
#    define ADC1_TIMER_PCLK_FREQUENCY LS_TIM2_CLKIN
#elif defined(CONFIG_LS_TIM6_ADC1)
#    define ADC1_HAVE_TIMER           1
#    define ADC1_TIMER_BASE           LS_TIM6_BASE
#    define ADC1_TIMER_PCLK_FREQUENCY LS_TIM6_CLKIN
#else
#    undef  ADC1_HAVE_TIMER
#endif

#ifdef ADC1_HAVE_TIMER
#  ifndef CONFIG_LS_ADC1_SAMPLE_FREQUENCY
#    error "CONFIG_LS_ADC1_SAMPLE_FREQUENCY not defined"
#  endif
#  ifndef CONFIG_LS_ADC1_TIMTRIG
#    error "CONFIG_LS_ADC1_TIMTRIG not defined"
#    warning "Values 0:CC1 1:CC2 2:CC3 3:CC4 4:TRGO 5:TRGO2"
#  endif
#endif

#ifdef ADC1_HAVE_TIMER
#  define ADC_HAVE_TIMER 1
#else
#  undef ADC_HAVE_TIMER
#endif

/* ADC interrupts ***********************************************************/

#define ADC_ISR_EOC                  ADC_SR_EOC
#define ADC_IER_EOC                  ADC_CR1_EOCIE
#define ADC_ISR_AWD                  ADC_SR_AWD
#define ADC_IER_AWD                  ADC_CR1_AWDIE
#define ADC_ISR_JEOC                 ADC_SR_JEOC
#define ADC_IER_JEOC                 ADC_CR1_JEOCIE

#define ADC_ISR_ALLINTS (ADC_ISR_EOC | ADC_ISR_AWD | ADC_ISR_JEOC)
#define ADC_IER_ALLINTS (ADC_IER_EOC | ADC_IER_AWD | ADC_IER_JEOC)

/* Low-level ops helpers ****************************************************/

#define LS_ADC_INT_ACK(adc, source)              \
        (adc)->llops->int_ack(adc, source)
#define LS_ADC_INT_GET(adc)                      \
        (adc)->llops->int_get(adc)
#define LS_ADC_INT_ENABLE(adc, source)           \
        (adc)->llops->int_en(adc, source)
#define LS_ADC_INT_DISABLE(adc, source)          \
        (adc)->llops->int_dis(adc, source)
#define LS_ADC_REGDATA_GET(adc)                  \
        (adc)->llops->val_get(adc)
#define LS_ADC_REGBUF_REGISTER(adc, buffer, len) \
        (adc)->llops->regbuf_reg(adc, buffer, len)
#define LS_ADC_REG_STARTCONV(adc, state)         \
        (adc)->llops->reg_startconv(adc, state)
#define LS_ADC_OFFSET_SET(adc, ch, i, o)         \
        (adc)->llops->offset_set(adc, ch, i, o)
#define LS_ADC_EXTCFG_SET(adc, c)                \
        (adc)->llops->extcfg_set(adc, c)
#define LS_ADC_INJ_STARTCONV(adc, state)         \
        (adc)->llops->inj_startconv(adc, state)
#define LS_ADC_INJDATA_GET(adc, chan)            \
        (adc)->llops->inj_get(adc, chan)
#define LS_ADC_JEXTCFG_SET(adc, c)               \
        (adc)->llops->jextcfg_set(adc, c)
#define LS_ADC_SAMPLETIME_SET(adc, time_samples) \
        (adc)->llops->stime_set(adc, time_samples)
#define LS_ADC_SAMPLETIME_WRITE(adc)             \
        (adc)->llops->stime_write(adc)
#define LS_ADC_DUMP_REGS(adc)                    \
        (adc)->llops->dump_regs(adc)
#define LS_ADC_SETUP(adc)                        \
        (adc)->llops->setup(adc)
#define LS_ADC_SHUTDOWN(adc)                     \
        (adc)->llops->shutdown(adc)
#define LS_ADC_ENABLE(adc, en)                   \
        (adc)->llops->enable(adc, en)

/****************************************************************************
 * Public Types
 ****************************************************************************/

enum adc_io_cmds_e
{
  IO_ENABLE_DISABLE_AWDIE,
  IO_ENABLE_DISABLE_EOCIE,
  IO_ENABLE_DISABLE_JEOCIE,
  IO_ENABLE_DISABLE_OVRIE,
  IO_ENABLE_DISABLE_ALL_INTS,
  IO_STOP_ADC,
  IO_START_ADC,
  IO_START_CONV,
  IO_TRIGGER_REG,
#ifdef ADC_HAVE_INJECTED
  IO_TRIGGER_INJ,
#endif
};

/* ADC resolution can be reduced in order to perform faster conversion */

enum ls_adc_resoluton_e
{
  ADC_RESOLUTION_12BIT = 0,     /* 12 bit */
  ADC_RESOLUTION_10BIT = 1,     /* 10 bit */
  ADC_RESOLUTION_8BIT  = 2,     /* 8 bit */
  ADC_RESOLUTION_6BIT  = 3      /* 6 bit */
};

#ifdef CONFIG_LS_ADC_LL_OPS

#ifdef CONFIG_LS_ADC_CHANGE_SAMPLETIME

/* Channel and sample time pair */

typedef struct adc_channel_s
{
  uint8_t channel:5;

  /* Sampling time individually for each channel.
   * It differs between families
   */

  uint8_t sample_time:3;
} adc_channel_t;

/* This structure will be used while setting channels to specified by the
 * "channel-sample time" pairs' values
 */

struct adc_sample_time_s
{
  adc_channel_t *channel;                /* Array of channels */
  uint8_t        channels_nbr:5;         /* Number of channels in array */
  bool           all_same:1;             /* All channels will get the
                                          * same value of the sample time */
  uint8_t        all_ch_sample_time:3;   /* Sample time for all channels */
};
#endif /* CONFIG_LS_ADC_CHANGE_SAMPLETIME */

/* This structure provides the publicly visible representation of the
 * "lower-half" ADC driver structure.
 */

struct ls_adc_dev_s
{
  /* Publicly visible portion of the "lower-half" ADC driver structure */

  const struct ls_adc_ops_s *llops;

  /* Require cast-compatibility with private "lower-half" ADC structure */
};

/* Low-level operations for ADC */

struct ls_adc_ops_s
{
  /* Low-level ADC setup */

  int (*setup)(struct ls_adc_dev_s *dev);

  /* Low-level ADC shutdown */

  void (*shutdown)(struct ls_adc_dev_s *dev);

  /* Acknowledge interrupts */

  void (*int_ack)(struct ls_adc_dev_s *dev, uint32_t source);

  /* Get pending interrupts */

  uint32_t (*int_get)(struct ls_adc_dev_s *dev);

  /* Enable interrupts */

  void (*int_en)(struct ls_adc_dev_s *dev, uint32_t source);

  /* Disable interrupts */

  void (*int_dis)(struct ls_adc_dev_s *dev, uint32_t source);

  /* Get current ADC data register */

  uint32_t (*val_get)(struct ls_adc_dev_s *dev);

  /* Register buffer for ADC DMA transfer */

  int (*regbuf_reg)(struct ls_adc_dev_s *dev,
                    uint16_t *buffer, uint8_t len);

  /* Start/stop regular conversion */

  void (*reg_startconv)(struct ls_adc_dev_s *dev, bool state);

  /* Set offset for channel */

  int (*offset_set)(struct ls_adc_dev_s *dev, uint8_t ch, uint8_t i,
                    uint16_t offset);

#ifdef ADC_HAVE_EXTCFG
  /* Configure the ADC external trigger for regular conversion */

  void (*extcfg_set)(struct ls_adc_dev_s *dev, uint32_t extcfg);
#endif

#ifdef ADC_HAVE_JEXTCFG
  /* Configure the ADC external trigger for injected conversion */

  void (*jextcfg_set)(struct ls_adc_dev_s *dev, uint32_t jextcfg);
#endif

#ifdef ADC_HAVE_INJECTED
  /* Get current ADC injected data register */

  uint32_t (*inj_get)(struct ls_adc_dev_s *dev, uint8_t chan);

  /* Start/stop injected conversion */

  void (*inj_startconv)(struct ls_adc_dev_s *dev, bool state);
#endif

#ifdef CONFIG_LS_ADC_CHANGE_SAMPLETIME
  /* Set ADC sample time */

  void (*stime_set)(struct ls_adc_dev_s *dev,
                    struct adc_sample_time_s *time_samples);

  /* Write ADC sample time */

  void (*stime_write)(struct ls_adc_dev_s *dev);
#endif

  void (*dump_regs)(struct ls_adc_dev_s *dev);

  /* Enable/disable ADC */

  void (*enable)(struct ls_adc_dev_s *dev, bool enable);
};

#endif /* CONFIG_LS_ADC_LL_OPS */

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__
#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: ls_adcinitialize
 *
 * Description:
 *   Initialize the ADC. See ls_adc.c for more details.
 *
 * Input Parameters:
 *   intf      - Could be {1} for ADC1
 *   chanlist  - The list of channels (regular + injected)
 *   nchannels - Number of channels (regular + injected)
 *
 * Returned Value:
 *   Valid ADC device structure reference on success; a NULL on failure
 *
 ****************************************************************************/

struct adc_dev_s;
struct adc_dev_s *ls_adcinitialize(int intf, const uint8_t *chanlist,
                                      int channels);

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif /* __ASSEMBLY__ */

#endif /* CONFIG_LS_ADC1 */
#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_ADC_H */
