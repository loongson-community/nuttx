/****************************************************************************
 * arch/loongarch/src/ls2k/hardware/ls_dma.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_DMA_H
#define __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_DMA_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* 1 DMA controller */

#define DMA1                      (0)

/* The LS2K030x family has 8 channels total:
 * 8 DMA1 channels(1-8).
 */

#define DMA_CHAN1                 (0)
#define DMA_CHAN2                 (1)
#define DMA_CHAN3                 (2)
#define DMA_CHAN4                 (3)
#define DMA_CHAN5                 (4)
#define DMA_CHAN6                 (5)
#define DMA_CHAN7                 (6)
#define DMA_CHAN8                 (7)

/* Register Offsets *********************************************************/

#define LS_DMA_ISR_OFFSET         0x0000 /* DMA interrupt status register */
#define LS_DMA_IFCR_OFFSET        0x0004 /* DMA interrupt flag clear register */

#define LS_DMACHAN_OFFSET(n)      (0x0014*(n))
#define LS_DMACHAN1_OFFSET        LS_DMACHAN_OFFSET(0)
#define LS_DMACHAN2_OFFSET        LS_DMACHAN_OFFSET(1)
#define LS_DMACHAN3_OFFSET        LS_DMACHAN_OFFSET(2)
#define LS_DMACHAN4_OFFSET        LS_DMACHAN_OFFSET(3)
#define LS_DMACHAN5_OFFSET        LS_DMACHAN_OFFSET(4)
#define LS_DMACHAN6_OFFSET        LS_DMACHAN_OFFSET(5)
#define LS_DMACHAN7_OFFSET        LS_DMACHAN_OFFSET(6)
#define LS_DMACHAN8_OFFSET        LS_DMACHAN_OFFSET(7)

#define LS_DMACHAN_CCR_OFFSET     0x0008 /* DMA channel configuration register */
#define LS_DMACHAN_CNDTR_OFFSET   0x000c /* DMA channel number of data register */
#define LS_DMACHAN_CPAR_OFFSET    0x0010 /* DMA channel peripheral address register */
#define LS_DMACHAN_CMAR_OFFSET    0x0014 /* DMA channel 1 memory address register */

#define LS_DMA_CCR_OFFSET(n)      (LS_DMACHAN_CCR_OFFSET+LS_DMACHAN_OFFSET(n))
#define LS_DMA_CNDTR_OFFSET(n)    (LS_DMACHAN_CNDTR_OFFSET+LS_DMACHAN_OFFSET(n))
#define LS_DMA_CPAR_OFFSET(n)     (LS_DMACHAN_CPAR_OFFSET+LS_DMACHAN_OFFSET(n))
#define LS_DMA_CMAR_OFFSET(n)     (LS_DMACHAN_CMAR_OFFSET+LS_DMACHAN_OFFSET(n))

#define LS_DMA_CCR1_OFFSET        LS_DMA_CCR_OFFSET(0) /* DMA channel 1 configuration register */
#define LS_DMA_CCR2_OFFSET        LS_DMA_CCR_OFFSET(1) /* DMA channel 2 configuration register */
#define LS_DMA_CCR3_OFFSET        LS_DMA_CCR_OFFSET(2) /* DMA channel 3 configuration register */
#define LS_DMA_CCR4_OFFSET        LS_DMA_CCR_OFFSET(3) /* DMA channel 4 configuration register */
#define LS_DMA_CCR5_OFFSET        LS_DMA_CCR_OFFSET(4) /* DMA channel 5 configuration register */
#define LS_DMA_CCR6_OFFSET        LS_DMA_CCR_OFFSET(5) /* DMA channel 6 configuration register */
#define LS_DMA_CCR7_OFFSET        LS_DMA_CCR_OFFSET(6) /* DMA channel 7 configuration register */
#define LS_DMA_CCR8_OFFSET        LS_DMA_CCR_OFFSET(7) /* DMA channel 8 configuration register */

#define LS_DMA_CNDTR1_OFFSET      LS_DMA_CNDTR_OFFSET(0) /* DMA channel 1 number of data register */
#define LS_DMA_CNDTR2_OFFSET      LS_DMA_CNDTR_OFFSET(1) /* DMA channel 2 number of data register */
#define LS_DMA_CNDTR3_OFFSET      LS_DMA_CNDTR_OFFSET(2) /* DMA channel 3 number of data register */
#define LS_DMA_CNDTR4_OFFSET      LS_DMA_CNDTR_OFFSET(3) /* DMA channel 4 number of data register */
#define LS_DMA_CNDTR5_OFFSET      LS_DMA_CNDTR_OFFSET(4) /* DMA channel 5 number of data register */
#define LS_DMA_CNDTR6_OFFSET      LS_DMA_CNDTR_OFFSET(5) /* DMA channel 6 number of data register */
#define LS_DMA_CNDTR7_OFFSET      LS_DMA_CNDTR_OFFSET(6) /* DMA channel 7 number of data register */
#define LS_DMA_CNDTR8_OFFSET      LS_DMA_CNDTR_OFFSET(7) /* DMA channel 8 number of data register */

#define LS_DMA_CPAR1_OFFSET       LS_DMA_CPAR_OFFSET(0) /* DMA channel 1 peripheral address register */
#define LS_DMA_CPAR2_OFFSET       LS_DMA_CPAR_OFFSET(1) /* DMA channel 2 peripheral address register */
#define LS_DMA_CPAR3_OFFSET       LS_DMA_CPAR_OFFSET(2) /* DMA channel 3 peripheral address register */
#define LS_DMA_CPAR4_OFFSET       LS_DMA_CPAR_OFFSET(3) /* DMA channel 4 peripheral address register */
#define LS_DMA_CPAR5_OFFSET       LS_DMA_CPAR_OFFSET(4) /* DMA channel 5 peripheral address register */
#define LS_DMA_CPAR6_OFFSET       LS_DMA_CPAR_OFFSET(5) /* DMA channel 6 peripheral address register */
#define LS_DMA_CPAR7_OFFSET       LS_DMA_CPAR_OFFSET(6) /* DMA channel 7 peripheral address register */
#define LS_DMA_CPAR8_OFFSET       LS_DMA_CPAR_OFFSET(7) /* DMA channel 8 peripheral address register */

#define LS_DMA_CMAR1_OFFSET       LS_DMA_CMAR_OFFSET(0) /* DMA channel 1 memory address register */
#define LS_DMA_CMAR2_OFFSET       LS_DMA_CMAR_OFFSET(1) /* DMA channel 2 memory address register */
#define LS_DMA_CMAR3_OFFSET       LS_DMA_CMAR_OFFSET(2) /* DMA channel 3 memory address register */
#define LS_DMA_CMAR4_OFFSET       LS_DMA_CMAR_OFFSET(3) /* DMA channel 4 memory address register */
#define LS_DMA_CMAR5_OFFSET       LS_DMA_CMAR_OFFSET(4) /* DMA channel 5 memory address register */
#define LS_DMA_CMAR6_OFFSET       LS_DMA_CMAR_OFFSET(5) /* DMA channel 6 memory address register */
#define LS_DMA_CMAR7_OFFSET       LS_DMA_CMAR_OFFSET(6) /* DMA channel 7 memory address register */
#define LS_DMA_CMAR8_OFFSET       LS_DMA_CMAR_OFFSET(7) /* DMA channel 8 memory address register */

/* Register Addresses *******************************************************/

#define LS_DMA1_ISRC              (LS_DMA1_BASE+LS_DMA_ISR_OFFSET)
#define LS_DMA1_IFCR              (LS_DMA1_BASE+LS_DMA_IFCR_OFFSET)

#define LS_DMA1_CCR(n)            (LS_DMA1_BASE+LS_DMA_CCR_OFFSET(n))
#define LS_DMA1_CCR1              (LS_DMA1_BASE+LS_DMA_CCR1_OFFSET)
#define LS_DMA1_CCR2              (LS_DMA1_BASE+LS_DMA_CCR2_OFFSET)
#define LS_DMA1_CCR3              (LS_DMA1_BASE+LS_DMA_CCR3_OFFSET)
#define LS_DMA1_CCR4              (LS_DMA1_BASE+LS_DMA_CCR4_OFFSET)
#define LS_DMA1_CCR5              (LS_DMA1_BASE+LS_DMA_CCR5_OFFSET)
#define LS_DMA1_CCR6              (LS_DMA1_BASE+LS_DMA_CCR6_OFFSET)
#define LS_DMA1_CCR7              (LS_DMA1_BASE+LS_DMA_CCR7_OFFSET)
#define LS_DMA1_CCR8              (LS_DMA1_BASE+LS_DMA_CCR8_OFFSET)

#define LS_DMA1_CNDTR(n)          (LS_DMA1_BASE+LS_DMA_CNDTR_OFFSET(n))
#define LS_DMA1_CNDTR1            (LS_DMA1_BASE+LS_DMA_CNDTR1_OFFSET)
#define LS_DMA1_CNDTR2            (LS_DMA1_BASE+LS_DMA_CNDTR2_OFFSET)
#define LS_DMA1_CNDTR3            (LS_DMA1_BASE+LS_DMA_CNDTR3_OFFSET)
#define LS_DMA1_CNDTR4            (LS_DMA1_BASE+LS_DMA_CNDTR4_OFFSET)
#define LS_DMA1_CNDTR5            (LS_DMA1_BASE+LS_DMA_CNDTR5_OFFSET)
#define LS_DMA1_CNDTR6            (LS_DMA1_BASE+LS_DMA_CNDTR6_OFFSET)
#define LS_DMA1_CNDTR7            (LS_DMA1_BASE+LS_DMA_CNDTR7_OFFSET)
#define LS_DMA1_CNDTR8            (LS_DMA1_BASE+LS_DMA_CNDTR8_OFFSET)

#define LS_DMA1_CPAR(n)           (LS_DMA1_BASE+LS_DMA_CPAR_OFFSET(n))
#define LS_DMA1_CPAR1             (LS_DMA1_BASE+LS_DMA_CPAR1_OFFSET)
#define LS_DMA1_CPAR2             (LS_DMA1_BASE+LS_DMA_CPAR2_OFFSET)
#define LS_DMA1_CPAR3             (LS_DMA1_BASE+LS_DMA_CPAR3_OFFSET)
#define LS_DMA1_CPAR4             (LS_DMA1_BASE+LS_DMA_CPAR4_OFFSET)
#define LS_DMA1_CPAR5             (LS_DMA1_BASE+LS_DMA_CPAR5_OFFSET)
#define LS_DMA1_CPAR6             (LS_DMA1_BASE+LS_DMA_CPAR6_OFFSET)
#define LS_DMA1_CPAR7             (LS_DMA1_BASE+LS_DMA_CPAR7_OFFSET)
#define LS_DMA1_CPAR8             (LS_DMA1_BASE+LS_DMA_CPAR8_OFFSET)

#define LS_DMA1_CMAR(n)           (LS_DMA1_BASE+LS_DMA_CMAR_OFFSET(n))
#define LS_DMA1_CMAR1             (LS_DMA1_BASE+LS_DMA_CMAR1_OFFSET)
#define LS_DMA1_CMAR2             (LS_DMA1_BASE+LS_DMA_CMAR2_OFFSET)
#define LS_DMA1_CMAR3             (LS_DMA1_BASE+LS_DMA_CMAR3_OFFSET)
#define LS_DMA1_CMAR4             (LS_DMA1_BASE+LS_DMA_CMAR4_OFFSET)
#define LS_DMA1_CMAR5             (LS_DMA1_BASE+LS_DMA_CMAR5_OFFSET)
#define LS_DMA1_CMAR6             (LS_DMA1_BASE+LS_DMA_CMAR6_OFFSET)
#define LS_DMA1_CMAR7             (LS_DMA1_BASE+LS_DMA_CMAR7_OFFSET)
#define LS_DMA1_CMAR8             (LS_DMA1_BASE+LS_DMA_CMAR8_OFFSET)

/* Register Bitfield Definitions ********************************************/

#define DMA_CHAN_SHIFT(n)         ((n) << 2)
#define DMA_CHAN_MASK             0x0f
#define DMA_CHAN_GIF_BIT          (1 << 0)  /* Bit 0: Channel Global interrupt flag */
#define DMA_CHAN_TCIF_BIT         (1 << 1)  /* Bit 1: Channel Transfer Complete flag */
#define DMA_CHAN_HTIF_BIT         (1 << 2)  /* Bit 2: Channel Half Transfer flag */
#define DMA_CHAN_TEIF_BIT         (1 << 3)  /* Bit 3: Channel Transfer Error flag */

/* DMA interrupt status register */

#define DMA_ISR_CHAN_SHIFT(n)     DMA_CHAN_SHIFT(n)
#define DMA_ISR_CHAN_MASK(n)      (DMA_CHAN_MASK <<  DMA_ISR_CHAN_SHIFT(n))
#define DMA_ISR_CHAN1_SHIFT       (0)       /* Bits 3-0:  DMA Channel 1 interrupt status */
#define DMA_ISR_CHAN1_MASK        (DMA_CHAN_MASK <<  DMA_ISR_CHAN1_SHIFT)
#define DMA_ISR_CHAN2_SHIFT       (4)       /* Bits 7-4:  DMA Channel 2 interrupt status */
#define DMA_ISR_CHAN2_MASK        (DMA_CHAN_MASK <<  DMA_ISR_CHAN2_SHIFT)
#define DMA_ISR_CHAN3_SHIFT       (8)       /* Bits 11-8:  DMA Channel 3 interrupt status */
#define DMA_ISR_CHAN3_MASK        (DMA_CHAN_MASK <<  DMA_ISR_CHAN3_SHIFT)
#define DMA_ISR_CHAN4_SHIFT       (12)      /* Bits 15-12:  DMA Channel 4 interrupt status */
#define DMA_ISR_CHAN4_MASK        (DMA_CHAN_MASK <<  DMA_ISR_CHAN4_SHIFT)
#define DMA_ISR_CHAN5_SHIFT       (16)      /* Bits 19-16:  DMA Channel 5 interrupt status */
#define DMA_ISR_CHAN5_MASK        (DMA_CHAN_MASK <<  DMA_ISR_CHAN5_SHIFT)
#define DMA_ISR_CHAN6_SHIFT       (20)      /* Bits 23-20:  DMA Channel 6 interrupt status */
#define DMA_ISR_CHAN6_MASK        (DMA_CHAN_MASK <<  DMA_ISR_CHAN6_SHIFT)
#define DMA_ISR_CHAN7_SHIFT       (24)      /* Bits 27-24:  DMA Channel 7 interrupt status */
#define DMA_ISR_CHAN7_MASK        (DMA_CHAN_MASK <<  DMA_ISR_CHAN7_SHIFT)
#define DMA_ISR_CHAN8_SHIFT       (28)      /* Bits 31-28:  DMA Channel 8 interrupt status */
#define DMA_ISR_CHAN8_MASK        (DMA_CHAN_MASK <<  DMA_ISR_CHAN8_SHIFT)

#define DMA_ISR_GIF(n)            (DMA_CHAN_GIF_BIT << DMA_ISR_CHAN_SHIFT(n))
#define DMA_ISR_TCIF(n)           (DMA_CHAN_TCIF_BIT << DMA_ISR_CHAN_SHIFT(n))
#define DMA_ISR_HTIF(n)           (DMA_CHAN_HTIF_BIT << DMA_ISR_CHAN_SHIFT(n))
#define DMA_ISR_TEIF(n)           (DMA_CHAN_TEIF_BIT << DMA_ISR_CHAN_SHIFT(n))

/* DMA interrupt flag clear register */

#define DMA_IFCR_CHAN_SHIFT(n)    DMA_CHAN_SHIFT(n)
#define DMA_IFCR_CHAN_MASK(n)     (DMA_CHAN_MASK <<  DMA_IFCR_CHAN_SHIFT(n))
#define DMA_IFCR_CHAN1_SHIFT      (0)       /* Bits 3-0:  DMA Channel 1 interrupt flag clear */
#define DMA_IFCR_CHAN1_MASK       (DMA_CHAN_MASK <<  DMA_IFCR_CHAN1_SHIFT)
#define DMA_IFCR_CHAN2_SHIFT      (4)       /* Bits 7-4:  DMA Channel 2 interrupt flag clear */
#define DMA_IFCR_CHAN2_MASK       (DMA_CHAN_MASK <<  DMA_IFCR_CHAN2_SHIFT)
#define DMA_IFCR_CHAN3_SHIFT      (8)       /* Bits 11-8:  DMA Channel 3 interrupt flag clear */
#define DMA_IFCR_CHAN3_MASK       (DMA_CHAN_MASK <<  DMA_IFCR_CHAN3_SHIFT)
#define DMA_IFCR_CHAN4_SHIFT      (12)      /* Bits 15-12:  DMA Channel 4 interrupt flag clear */
#define DMA_IFCR_CHAN4_MASK       (DMA_CHAN_MASK <<  DMA_IFCR_CHAN4_SHIFT)
#define DMA_IFCR_CHAN5_SHIFT      (16)      /* Bits 19-16:  DMA Channel 5 interrupt flag clear */
#define DMA_IFCR_CHAN5_MASK       (DMA_CHAN_MASK <<  DMA_IFCR_CHAN5_SHIFT)
#define DMA_IFCR_CHAN6_SHIFT      (20)      /* Bits 23-20:  DMA Channel 6 interrupt flag clear */
#define DMA_IFCR_CHAN6_MASK       (DMA_CHAN_MASK <<  DMA_IFCR_CHAN6_SHIFT)
#define DMA_IFCR_CHAN7_SHIFT      (24)      /* Bits 27-24:  DMA Channel 7 interrupt flag clear */
#define DMA_IFCR_CHAN7_MASK       (DMA_CHAN_MASK <<  DMA_IFCR_CHAN7_SHIFT)
#define DMA_IFCR_CHAN8_SHIFT      (28)      /* Bits 31-28:  DMA Channel 8 interrupt flag clear */
#define DMA_IFCR_CHAN8_MASK       (DMA_CHAN_MASK <<  DMA_IFCR_CHAN8_SHIFT)

#define DMA_IFCR_ALLCHANNELS      (0xffffffff)

#define DMA_IFCR_CGIF(n)          (DMA_CHAN_GIF_BIT << DMA_IFCR_CHAN_SHIFT(n))
#define DMA_IFCR_CTCIF(n)         (DMA_CHAN_TCIF_BIT << DMA_IFCR_CHAN_SHIFT(n))
#define DMA_IFCR_CHTIF(n)         (DMA_CHAN_HTIF_BIT << DMA_IFCR_CHAN_SHIFT(n))
#define DMA_IFCR_CTEIF(n)         (DMA_CHAN_TEIF_BIT << DMA_IFCR_CHAN_SHIFT(n))

/* DMA channel configuration register */

#define DMA_CCR_EN                (1 << 0)                   /* Bit 0: Channel enable */
#define DMA_CCR_TCIE              (1 << 1)                   /* Bit 1: Transfer complete interrupt enable */
#define DMA_CCR_HTIE              (1 << 2)                   /* Bit 2: Half Transfer interrupt enable */
#define DMA_CCR_TEIE              (1 << 3)                   /* Bit 3: Transfer error interrupt enable */
#define DMA_CCR_DIR               (1 << 4)                   /* Bit 4: Data transfer direction */
#define DMA_CCR_CIRC              (1 << 5)                   /* Bit 5: Circular mode */
#define DMA_CCR_PINC              (1 << 6)                   /* Bit 6: Peripheral increment mode */
#define DMA_CCR_MINC              (1 << 7)                   /* Bit 7: Memory increment mode */
#define DMA_CCR_PSIZE_SHIFT       (8)                        /* Bits 8-9: Peripheral size */
#define DMA_CCR_PSIZE_MASK        (3 << DMA_CCR_PSIZE_SHIFT)
#  define DMA_CCR_PSIZE_8BITS     (0 << DMA_CCR_PSIZE_SHIFT) /* 00: 8-bits */
#  define DMA_CCR_PSIZE_16BITS    (1 << DMA_CCR_PSIZE_SHIFT) /* 01: 16-bits */
#  define DMA_CCR_PSIZE_32BITS    (2 << DMA_CCR_PSIZE_SHIFT) /* 10: 32-bits */
#define DMA_CCR_MSIZE_SHIFT       (10)                       /* Bits 10-11: Memory size */
#define DMA_CCR_MSIZE_MASK        (3 << DMA_CCR_MSIZE_SHIFT)
#  define DMA_CCR_MSIZE_8BITS     (0 << DMA_CCR_MSIZE_SHIFT) /* 00: 8-bits */
#  define DMA_CCR_MSIZE_16BITS    (1 << DMA_CCR_MSIZE_SHIFT) /* 01: 16-bits */
#  define DMA_CCR_MSIZE_32BITS    (2 << DMA_CCR_MSIZE_SHIFT) /* 10: 32-bits */
#define DMA_CCR_PL_SHIFT          (12)                       /* Bits 12-13: Channel Priority level */
#define DMA_CCR_PL_MASK           (3 << DMA_CCR_PL_SHIFT)
#  define DMA_CCR_PRILO           (0 << DMA_CCR_PL_SHIFT)    /* 00: Low */
#  define DMA_CCR_PRIMED          (1 << DMA_CCR_PL_SHIFT)    /* 01: Medium */
#  define DMA_CCR_PRIHI           (2 << DMA_CCR_PL_SHIFT)    /* 10: High */
#  define DMA_CCR_PRIVERYHI       (3 << DMA_CCR_PL_SHIFT)    /* 11: Very high */
#define DMA_CCR_MEM2MEM           (1 << 14)                  /* Bit 14: Memory to memory mode */

#define DMA_CCR_ALLINTS           (DMA_CCR_TEIE|DMA_CCR_HTIE|DMA_CCR_TCIE)

/* DMA channel number of data register */

#define DMA_CNDTR_NDT_SHIFT       (0)       /* Bits 31-0: Number of data to Transfer */
#define DMA_CNDTR_NDT_MASK        (0xffffffff << DMA_CNDTR_NDT_SHIFT)

/* DMA Channel mapping.
 * Each DMA channel has a mapping to several possible  sources/sinks of data.
 * The requests from peripherals assigned to a channel are simply OR'ed
 * together before entering the DMA block.  This means that onlyone request
 * on a given channel can be enabled at once.
 *
 * Alternative DMA channel selections are provided with a numeric suffix like
 * _1, _2, etc.  Drivers, however, will use the pin selection without the
 * numeric suffix. Additional definitions are required in the board.h file.
 */

#define LS_DMA1_CHAN1             (0)
#define LS_DMA1_CHAN2             (1)
#define LS_DMA1_CHAN3             (2)
#define LS_DMA1_CHAN4             (3)
#define LS_DMA1_CHAN5             (4)
#define LS_DMA1_CHAN6             (5)
#define LS_DMA1_CHAN7             (6)
#define LS_DMA1_CHAN8             (7)

#endif /* __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_DMA_H */
