/****************************************************************************
 * arch/loongarch/src/ls2k/hardware/ls_adc.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_ADC_H
#define __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_ADC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "chip.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* LS2K0300 has the basic version of ADC hardware
 *   - no common ADC registers
 *   - ADCs are not coupled in single ADC block (no common ADC base address)
 *   - no configurable resolution
 *   - no overrun
 *   - ...
 */

/* Register Offsets *********************************************************/

#define LS_ADC_SR_OFFSET          0x0000  /* ADC status register (32-bit) */
#define LS_ADC_CR1_OFFSET         0x0004  /* ADC control register 1 (32-bit) */
#define LS_ADC_CR2_OFFSET         0x0008  /* ADC control register 2 (32-bit) */
#define LS_ADC_SMPR1_OFFSET       0x000c  /* ADC sample time register 1 (32-bit) */
#define LS_ADC_SMPR2_OFFSET       0x0010  /* ADC sample time register 2 (32-bit) */
#define LS_ADC_JOFR1_OFFSET       0x0014  /* ADC injected channel data offset register 1 (32-bit) */
#define LS_ADC_JOFR2_OFFSET       0x0018  /* ADC injected channel data offset register 2 (32-bit) */
#define LS_ADC_JOFR3_OFFSET       0x001c  /* ADC injected channel data offset register 3 (32-bit) */
#define LS_ADC_JOFR4_OFFSET       0x0020  /* ADC injected channel data offset register 4 (32-bit) */
#define LS_ADC_HTR_OFFSET         0x0024  /* ADC watchdog high threshold register (32-bit) */
#define LS_ADC_LTR_OFFSET         0x0028  /* ADC watchdog low threshold register (32-bit) */
#define LS_ADC_SQR1_OFFSET        0x002c  /* ADC regular sequence register 1 (32-bit) */
#define LS_ADC_SQR2_OFFSET        0x0030  /* ADC regular sequence register 2 (32-bit) */
#define LS_ADC_SQR3_OFFSET        0x0034  /* ADC regular sequence register 3 (32-bit) */
#define LS_ADC_JSQR_OFFSET        0x0038  /* ADC injected sequence register (32-bit) */
#define LS_ADC_JDR1_OFFSET        0x003c  /* ADC injected data register 1 (32-bit) */
#define LS_ADC_JDR2_OFFSET        0x0040  /* ADC injected data register 1 (32-bit) */
#define LS_ADC_JDR3_OFFSET        0x0044  /* ADC injected data register 1 (32-bit) */
#define LS_ADC_JDR4_OFFSET        0x0048  /* ADC injected data register 1 (32-bit) */
#define LS_ADC_DR_OFFSET          0x004c  /* ADC regular data register (32-bit) */

/* Register Addresses *******************************************************/

#if LS_NADC > 0
#  define LS_ADC1_SR              (LS_ADC1_BASE + LS_ADC_SR_OFFSET)
#  define LS_ADC1_CR1             (LS_ADC1_BASE + LS_ADC_CR1_OFFSET)
#  define LS_ADC1_CR2             (LS_ADC1_BASE + LS_ADC_CR2_OFFSET)
#  define LS_ADC1_SMPR1           (LS_ADC1_BASE + LS_ADC_SMPR1_OFFSET)
#  define LS_ADC1_SMPR2           (LS_ADC1_BASE + LS_ADC_SMPR2_OFFSET)
#  define LS_ADC1_JOFR1           (LS_ADC1_BASE + LS_ADC_JOFR1_OFFSET)
#  define LS_ADC1_JOFR2           (LS_ADC1_BASE + LS_ADC_JOFR2_OFFSET)
#  define LS_ADC1_JOFR3           (LS_ADC1_BASE + LS_ADC_JOFR3_OFFSET)
#  define LS_ADC1_JOFR4           (LS_ADC1_BASE + LS_ADC_JOFR4_OFFSET)
#  define LS_ADC1_HTR             (LS_ADC1_BASE + LS_ADC_HTR_OFFSET)
#  define LS_ADC1_LTR             (LS_ADC1_BASE + LS_ADC_LTR_OFFSET)
#  define LS_ADC1_SQR1            (LS_ADC1_BASE + LS_ADC_SQR1_OFFSET)
#  define LS_ADC1_SQR2            (LS_ADC1_BASE + LS_ADC_SQR2_OFFSET)
#  define LS_ADC1_SQR3            (LS_ADC1_BASE + LS_ADC_SQR3_OFFSET)
#  define LS_ADC1_JSQR            (LS_ADC1_BASE + LS_ADC_JSQR_OFFSET)
#  define LS_ADC1_JDR1            (LS_ADC1_BASE + LS_ADC_JDR1_OFFSET)
#  define LS_ADC1_JDR2            (LS_ADC1_BASE + LS_ADC_JDR2_OFFSET)
#  define LS_ADC1_JDR3            (LS_ADC1_BASE + LS_ADC_JDR3_OFFSET)
#  define LS_ADC1_JDR4            (LS_ADC1_BASE + LS_ADC_JDR4_OFFSET)
#  define LS_ADC1_DR              (LS_ADC1_BASE + LS_ADC_DR_OFFSET)
#endif

/* Register Bitfield Definitions ********************************************/

/* ADC status register */

#define ADC_SR_AWD                   (1 << 0)  /* Bit 0 : Analog watchdog flag */
#define ADC_SR_EOC                   (1 << 1)  /* Bit 1 : End of conversion */
#define ADC_SR_JEOC                  (1 << 2)  /* Bit 2 : Injected channel end of conversion */
#define ADC_SR_JSTRT                 (1 << 3)  /* Bit 3 : Injected channel Start flag */
#define ADC_SR_STRT                  (1 << 4)  /* Bit 4 : Regular channel Start flag */

/* ADC control register 1 */

#define ADC_CR1_AWDCH_SHIFT          (0)       /* Bits 4-0: Analog watchdog channel select bits */
#define ADC_CR1_AWDCH_MASK           (0x1f << ADC_CR1_AWDCH_SHIFT)

#define ADC_CR1_EOCIE                (1 << 5)  /* Bit 5: Interrupt enable for EOC */
#define ADC_CR1_AWDIE                (1 << 6)  /* Bit 6: Analog Watchdog interrupt enable */
#define ADC_CR1_JEOCIE               (1 << 7)  /* Bit 7: Interrupt enable for injected channels */
#define ADC_CR1_SCAN                 (1 << 8)  /* Bit 8: Scan mode */
#define ADC_CR1_AWDSGL               (1 << 9)  /* Bit 9: Enable the watchdog on a single channel in scan mode */
#define ADC_CR1_JAUTO                (1 << 10) /* Bit 10: Automatic Injected Group conversion */
#define ADC_CR1_DISCEN               (1 << 11) /* Bit 11: Discontinuous mode on regular channels */
#define ADC_CR1_JDISCEN              (1 << 12) /* Bit 12: Discontinuous mode on injected channels */

#define ADC_CR1_DISCNUM_SHIFT        (13)      /* Bits 15-13: Discontinuous mode channel count */
#define ADC_CR1_DISCNUM_MASK         (0x07 << ADC_CR1_DISCNUM_SHIFT)

#define ADC_CR1_JAWDEN               (1 << 22) /* Bit 22: Analog watchdog enable on injected channels */
#define ADC_CR1_AWDEN                (1 << 23) /* Bit 23: Analog watchdog enable on regular channels */
#define ADC_CR1_CLKDIV_SHIFT         24
#define ADC_CR1_CLKDIV_MASK          (0x3f << ADC_CR1_CLKDIV_SHIFT)

/* ADC control register 2 */

#define ADC_CR2_ADON                 (1 << 0)  /* Bit 0: A/D Converter ON / OFF */
#define ADC_CR2_CONT                 (1 << 1)  /* Bit 1: Continuous Conversion */
#define ADC_CR2_CAL                  (1 << 2)  /* Bit 2: A/D Calibration */
#define ADC_CR2_RSTCAL               (1 << 3)  /* Bit 3: Reset Calibration */
#define ADC_CR2_DMA                  (1 << 8)  /* Bit 8: Direct Memory access mode */

#define ADC_CR2_ALIGN                (1 << 11) /* Bit 11: Data Alignment */

#define ADC_CR2_JEXTSEL_SHIFT        (12)      /* Bits 12-14: External event select for injected group */
#define ADC_CR2_JEXTSEL_MASK         (7 << ADC_CR2_JEXTSEL_SHIFT)
#define ADC_CR2_JEXTSEL_T1TRGO       (0 << ADC_CR2_JEXTSEL_SHIFT) /* 000: Timer 1 TRGO event */
#define ADC_CR2_JEXTSEL_T1CC4        (1 << ADC_CR2_JEXTSEL_SHIFT) /* 001: Timer 1 CC4 event */
#define ADC_CR2_JEXTSEL_T2TRGO       (2 << ADC_CR2_JEXTSEL_SHIFT) /* 010: Timer 2 TRGO event */
#define ADC_CR2_JEXTSEL_T2CC1        (3 << ADC_CR2_JEXTSEL_SHIFT) /* 011: Timer 2 CC1 event */
#define ADC_CR2_JEXTSEL_EXTI15       (6 << ADC_CR2_JEXTSEL_SHIFT) /* 110: EXTI line 15 (GPIO 77/101 Mul Func 2) */
#define ADC_CR2_JEXTSEL_JSWSTART     (7 << ADC_CR2_JEXTSEL_SHIFT) /* 111: JSWSTART */

#define ADC_CR2_JEXTTRIG             (1 << 15) /* Bit 15: External Trigger Conversion mode for injected channels */
#define ADC_CR2_EXTSEL_SHIFT         (17)      /* Bits 19-17: External Event Select for regular group */
#define ADC_CR2_EXTSEL_MASK          (7 << ADC_CR2_EXTSEL_SHIFT)
#define ADC_CR2_EXTSEL_T1CC1         (0 << ADC_CR2_EXTSEL_SHIFT) /* 000: Timer 1 CC1 event */
#define ADC_CR2_EXTSEL_T1CC2         (1 << ADC_CR2_EXTSEL_SHIFT) /* 001: Timer 1 CC2 event */
#define ADC_CR2_EXTSEL_T1CC3         (2 << ADC_CR2_EXTSEL_SHIFT) /* 010: Timer 1 CC3 event */
#define ADC_CR2_EXTSEL_T2CC2         (3 << ADC_CR2_EXTSEL_SHIFT) /* 011: Timer 2 CC2 event */
#define ADC_CR2_EXTSEL_EXTI11        (6 << ADC_CR2_EXTSEL_SHIFT) /* 110: EXTI line 11 (GPIO 76/100 Mul Func 2) */
#define ADC_CR2_EXTSEL_SWSTART       (7 << ADC_CR2_EXTSEL_SHIFT) /* 111: SWSTART */

#define ADC_CR2_EXTTRIG              (1 << 20) /* Bit 20: External Trigger Conversion mode for regular channels */
#define ADC_CR2_JSWSTART             (1 << 21) /* Bit 21: Start Conversion of injected channels */
#define ADC_CR2_SWSTART              (1 << 22) /* Bit 22: Start Conversion of regular channels */
#define ADC_CR2_ADCEDGE              (1 << 30)
#define ADC_CR2_CLKDIV_HI_SHIFT      26
#define ADC_CR2_CLKDIV_HI_MASK       (0x0f << ADC_CR2_CLKDIV_HI_SHIFT)

/* ADC sample time register 1 */

#define ADC_SMPR_1                   0         /* 000: 1 cycle */
#define ADC_SMPR_2                   1         /* 001: 2 cycles */
#define ADC_SMPR_4                   2         /* 010: 4 cycles */
#define ADC_SMPR_8                   3         /* 011: 8 cycles */
#define ADC_SMPR_16                  4         /* 100: 16 cycles */
#define ADC_SMPR_32                  5         /* 101: 32 cycles */
#define ADC_SMPR_64                  6         /* 110: 64 cycles */
#define ADC_SMPR_128                 7         /* 111: 128 cycles */

#define ADC_SMPR1_SMP10_SHIFT        (0)       /* Bits 0-2: Channel 10 Sample time selection */
#define ADC_SMPR1_SMP10_MASK         (7 << ADC_SMPR1_SMP10_SHIFT)
#define ADC_SMPR1_SMP11_SHIFT        (3)       /* Bits 3-5: Channel 11 Sample time selection */
#define ADC_SMPR1_SMP11_MASK         (7 << ADC_SMPR1_SMP11_SHIFT)
#define ADC_SMPR1_SMP12_SHIFT        (6)       /* Bits 6-8: Channel 12 Sample time selection */
#define ADC_SMPR1_SMP12_MASK         (7 << ADC_SMPR1_SMP12_SHIFT)
#define ADC_SMPR1_SMP13_SHIFT        (9)       /* Bits 9-11: Channel 13 Sample time selection */
#define ADC_SMPR1_SMP13_MASK         (7 << ADC_SMPR1_SMP13_SHIFT)
#define ADC_SMPR1_SMP14_SHIFT        (12)      /* Bits 12-14: Channel 14 Sample time selection */
#define ADC_SMPR1_SMP14_MASK         (7 << ADC_SMPR1_SMP14_SHIFT)
#define ADC_SMPR1_SMP15_SHIFT        (15)      /* Bits 15-17: Channel 15 Sample time selection */
#define ADC_SMPR1_SMP15_MASK         (7 << ADC_SMPR1_SMP15_SHIFT)
#define ADC_SMPR1_SMP16_SHIFT        (18)      /* Bits 18-20: Channel 16 Sample time selection */
#define ADC_SMPR1_SMP16_MASK         (7 << ADC_SMPR1_SMP16_SHIFT)
#define ADC_SMPR1_SMP17_SHIFT        (21)      /* Bits 21-23: Channel 17 Sample time selection */
#define ADC_SMPR1_SMP17_MASK         (7 << ADC_SMPR1_SMP17_SHIFT)

/* ADC sample time register 2 */

#define ADC_SMPR2_SMP0_SHIFT         (0)       /* Bits 2-0: Channel 0 Sample time selection */
#define ADC_SMPR2_SMP0_MASK          (7 << ADC_SMPR2_SMP0_SHIFT)
#define ADC_SMPR2_SMP1_SHIFT         (3)       /* Bits 5-3: Channel 1 Sample time selection */
#define ADC_SMPR2_SMP1_MASK          (7 << ADC_SMPR2_SMP1_SHIFT)
#define ADC_SMPR2_SMP2_SHIFT         (6)       /* Bits 8-6: Channel 2 Sample time selection */
#define ADC_SMPR2_SMP2_MASK          (7 << ADC_SMPR2_SMP2_SHIFT)
#define ADC_SMPR2_SMP3_SHIFT         (9)       /* Bits 11-9: Channel 3 Sample time selection */
#define ADC_SMPR2_SMP3_MASK          (7 << ADC_SMPR2_SMP3_SHIFT)
#define ADC_SMPR2_SMP4_SHIFT         (12)      /* Bits 14-12: Channel 4 Sample time selection */
#define ADC_SMPR2_SMP4_MASK          (7 << ADC_SMPR2_SMP4_SHIFT)
#define ADC_SMPR2_SMP5_SHIFT         (15)      /* Bits 17-15: Channel 5 Sample time selection */
#define ADC_SMPR2_SMP5_MASK          (7 << ADC_SMPR2_SMP5_SHIFT)
#define ADC_SMPR2_SMP6_SHIFT         (18)      /* Bits 20-18: Channel 6 Sample time selection */
#define ADC_SMPR2_SMP6_MASK          (7 << ADC_SMPR2_SMP6_SHIFT)
#define ADC_SMPR2_SMP7_SHIFT         (21)      /* Bits 23-21: Channel 7 Sample time selection */
#define ADC_SMPR2_SMP7_MASK          (7 << ADC_SMPR2_SMP7_SHIFT)
#define ADC_SMPR2_SMP8_SHIFT         (24)      /* Bits 26-24: Channel 8 Sample time selection */
#define ADC_SMPR2_SMP8_MASK          (7 << ADC_SMPR2_SMP8_SHIFT)
#define ADC_SMPR2_SMP9_SHIFT         (27)      /* Bits 29-27: Channel 9 Sample time selection */
#define ADC_SMPR2_SMP9_MASK          (7 << ADC_SMPR2_SMP9_SHIFT)

/* ADC injected channel data offset register 1-4 */

#define ADC_JOFR_SHIFT               (0)       /* Bits 11-0: Data offset for injected channel x */
#define ADC_JOFR_MASK                (0x0fff << ADC_JOFR_SHIFT)

/* ADC watchdog high threshold register */

#define ADC_HTR_SHIFT                (0)       /* Bits 11-0: Analog watchdog high threshold */
#define ADC_HTR_MASK                 (0x0fff << ADC_HTR_SHIFT)

/* ADC watchdog low threshold register */

#define ADC_LTR_SHIFT                (0)       /* Bits 11-0: Analog watchdog low threshold */
#define ADC_LTR_MASK                 (0x0fff << ADC_LTR_SHIFT)

/* ADC regular sequence register 1 */

#define ADC_SQR1_SQ13_SHIFT        (0)       /* Bits 4-0: 13th conversion in regular sequence */
#define ADC_SQR1_SQ13_MASK         (0x1f << ADC_SQR1_SQ13_SHIFT)
#define ADC_SQR1_SQ14_SHIFT        (5)       /* Bits 9-5: 14th conversion in regular sequence */
#define ADC_SQR1_SQ14_MASK         (0x1f << ADC_SQR1_SQ14_SHIFT)
#define ADC_SQR1_SQ15_SHIFT        (10)      /* Bits 14-10: 15th conversion in regular sequence */
#define ADC_SQR1_SQ15_MASK         (0x1f << ADC_SQR1_SQ15_SHIFT)
#define ADC_SQR1_SQ16_SHIFT        (15)      /* Bits 19-15: 16th conversion in regular sequence */
#define ADC_SQR1_SQ16_MASK         (0x1f << ADC_SQR1_SQ16_SHIFT)
#define ADC_SQR1_L_SHIFT           (20)      /* Bits 23-20: Regular channel sequence length */
#define ADC_SQR1_L_MASK            (0x0f << ADC_SQR1_L_SHIFT)
#define ADC_SQR1_RESERVED          (0xff000000)
#define ADC_SQR1_FIRST             (13)
#define ADC_SQR1_LAST              (16)
#define ADC_SQR1_SQ_OFFSET         (0)

/* ADC regular sequence register 2 */

#define ADC_SQR2_SQ7_SHIFT         (0)       /* Bits 4-0: 7th conversion in regular sequence */
#define ADC_SQR2_SQ7_MASK          (0x1f << ADC_SQR2_SQ7_SHIFT)
#define ADC_SQR2_SQ8_SHIFT         (5)       /* Bits 9-5: 8th conversion in regular sequence */
#define ADC_SQR2_SQ8_MASK          (0x1f << ADC_SQR2_SQ8_SHIFT)
#define ADC_SQR2_SQ9_SHIFT         (10)      /* Bits 14-10: 9th conversion in regular sequence */
#define ADC_SQR2_SQ9_MASK          (0x1f << ADC_SQR2_SQ9_SHIFT)
#define ADC_SQR2_SQ10_SHIFT        (15)      /* Bits 19-15: 10th conversion in regular sequence */
#define ADC_SQR2_SQ10_MASK         (0x1f << ADC_SQR2_SQ10_SHIFT)
#define ADC_SQR2_SQ11_SHIFT        (20)      /* Bits 24-20: 11th conversion in regular sequence */
#define ADC_SQR2_SQ11_MASK         (0x1f << ADC_SQR2_SQ11_SHIFT )
#define ADC_SQR2_SQ12_SHIFT        (25)      /* Bits 29-25: 12th conversion in regular sequence */
#define ADC_SQR2_SQ12_MASK         (0x1f << ADC_SQR2_SQ12_SHIFT)
#define ADC_SQR2_RESERVED          (0xc0000000)
#define ADC_SQR2_FIRST             (7)
#define ADC_SQR2_LAST              (12)
#define ADC_SQR2_SQ_OFFSET         (0)

/* ADC regular sequence register 3 */

#define ADC_SQR3_SQ1_SHIFT         (0)       /* Bits 4-0: 1st conversion in regular sequence */
#define ADC_SQR3_SQ1_MASK          (0x1f << ADC_SQR3_SQ1_SHIFT)
#define ADC_SQR3_SQ2_SHIFT         (5)       /* Bits 9-5: 2nd conversion in regular sequence */
#define ADC_SQR3_SQ2_MASK          (0x1f << ADC_SQR3_SQ2_SHIFT)
#define ADC_SQR3_SQ3_SHIFT         (10)      /* Bits 14-10: 3rd conversion in regular sequence */
#define ADC_SQR3_SQ3_MASK          (0x1f << ADC_SQR3_SQ3_SHIFT)
#define ADC_SQR3_SQ4_SHIFT         (15)      /* Bits 19-15: 4th conversion in regular sequence */
#define ADC_SQR3_SQ4_MASK          (0x1f << ADC_SQR3_SQ4_SHIFT)
#define ADC_SQR3_SQ5_SHIFT         (20)      /* Bits 24-20: 5th conversion in regular sequence */
#define ADC_SQR3_SQ5_MASK          (0x1f << ADC_SQR3_SQ5_SHIFT )
#define ADC_SQR3_SQ6_SHIFT         (25)      /* Bits 29-25: 6th conversion in regular sequence */
#define ADC_SQR3_SQ6_MASK          (0x1f << ADC_SQR3_SQ6_SHIFT)
#define ADC_SQR3_RESERVED          (0xc0000000)
#define ADC_SQR3_FIRST             (1)
#define ADC_SQR3_LAST              (6)
#define ADC_SQR3_SQ_OFFSET         (0)

/* Offset between SQ bits */

#define ADC_SQ_OFFSET                (5)

/* ADC injected sequence register */

#define ADC_JSQR_JSQ1_SHIFT          (0)       /* Bits 4-0: 1st conversion in injected sequence */
#define ADC_JSQR_JSQ1_MASK           (0x1f << ADC_JSQR_JSQ1_SHIFT)
#define ADC_JSQR_JSQ2_SHIFT          (5)       /* Bits 9-5: 2nd conversion in injected sequence */
#define ADC_JSQR_JSQ2_MASK           (0x1f << ADC_JSQR_JSQ2_SHIFT)
#define ADC_JSQR_JSQ3_SHIFT          (10)      /* Bits 14-10: 3rd conversion in injected sequence */
#define ADC_JSQR_JSQ3_MASK           (0x1f << ADC_JSQR_JSQ3_SHIFT)
#define ADC_JSQR_JSQ4_SHIFT          (15)      /* Bits 19-15: 4th conversion in injected sequence */
#define ADC_JSQR_JSQ4_MASK           (0x1f << ADC_JSQR_JSQ4_SHIFT)
#define ADC_JSQR_JSQ_SHIFT           (5)       /* Shift between JSQx bits */
#define ADC_JSQR_JL_SHIFT            (20)      /* Bits 21-20: Injected Sequence length */
#define ADC_JSQR_JL_MASK             (3 << ADC_JSQR_JL_SHIFT)
#  define ADC_JSQR_JL(n)             (((n)-1) << ADC_JSQR_JL_SHIFT) /* n=1..4 */

/* ADC injected data register 1-4 */

#define ADC_JDR_JDATA_SHIFT          (0)       /* Bits 15-0: Injected data */
#define ADC_JDR_JDATA_MASK           (0xffff << ADC_JDR_JDATA_SHIFT)

/* ADC regular data register */

#define ADC_DR_RDATA_SHIFT           (0)       /* Bits 15-0 Regular data */
#define ADC_DR_RDATA_MASK            (0xffff << ADC_DR_RDATA_SHIFT)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#endif /* __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_ADC_H */
