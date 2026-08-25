/****************************************************************************
 * arch/loongarch/src/ls2k/ls_dma.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <nuttx/debug.h>
#include <errno.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/semaphore.h>

#include "loongarch_internal.h"
#include "sched/sched.h"
#include "chip.h"
#include "ls_dma.h"
#include "ls.h"

/* This file is ported from the STM32 DMA IP core version 1 driver.
 * arch/arm/src/common/stm32/stm32_dma_m3m4_v1_8ch.c
 */

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DMA1_NCHANNELS        8
#define DMA_NCHANNELS         DMA1_NCHANNELS

/* Convert the DMA channel base address to the DMA register block address */

#define DMA_BASE(ch)          LS_DMA1_BASE

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure describes one DMA channel */

struct ls_dma_s
{
  uint8_t        chan;     /* DMA channel number (0-7) */
  uint8_t        irq;      /* DMA channel IRQ number */
  sem_t          sem;      /* Used to wait for DMA channel to become available */
  uintptr_t      base;     /* DMA register channel base address */
  dma_callback_t callback; /* Callback invoked when the DMA completes */
  void          *arg;      /* Argument passed to callback function */
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* This array describes the state of each DMA */

static struct ls_dma_s g_dma[DMA_NCHANNELS] =
{
#if DMA1_NCHANNELS > 0
  {
    .chan     = 0,
    .irq      = LS_IRQ_DMA1CH1,
    .sem      = SEM_INITIALIZER(1),
    .base     = LS_DMA1_BASE + LS_DMACHAN_OFFSET(0),
  },
#endif /* DMA1_NCHANNELS > 0 */
#if DMA1_NCHANNELS > 1
  {
    .chan     = 1,
    .irq      = LS_IRQ_DMA1CH2,
    .sem      = SEM_INITIALIZER(1),
    .base     = LS_DMA1_BASE + LS_DMACHAN_OFFSET(1),
  },
#endif /* DMA1_NCHANNELS > 1 */
#if DMA1_NCHANNELS > 2
  {
    .chan     = 2,
    .irq      = LS_IRQ_DMA1CH3,
    .sem      = SEM_INITIALIZER(1),
    .base     = LS_DMA1_BASE + LS_DMACHAN_OFFSET(2),
  },
#endif /* DMA1_NCHANNELS > 2 */
#if DMA1_NCHANNELS > 3
  {
    .chan     = 3,
    .irq      = LS_IRQ_DMA1CH4,
    .sem      = SEM_INITIALIZER(1),
    .base     = LS_DMA1_BASE + LS_DMACHAN_OFFSET(3),
  },
#endif /* DMA1_NCHANNELS > 3 */
#if DMA1_NCHANNELS > 4
  {
    .chan     = 4,
    .irq      = LS_IRQ_DMA1CH5,
    .sem      = SEM_INITIALIZER(1),
    .base     = LS_DMA1_BASE + LS_DMACHAN_OFFSET(4),
  },
#endif /* DMA1_NCHANNELS > 4 */
#if DMA1_NCHANNELS > 5
  {
    .chan     = 5,
    .irq      = LS_IRQ_DMA1CH6,
    .sem      = SEM_INITIALIZER(1),
    .base     = LS_DMA1_BASE + LS_DMACHAN_OFFSET(5),
  },
#endif /* DMA1_NCHANNELS > 5 */
#if DMA1_NCHANNELS > 6
  {
    .chan     = 6,
    .irq      = LS_IRQ_DMA1CH7,
    .sem      = SEM_INITIALIZER(1),
    .base     = LS_DMA1_BASE + LS_DMACHAN_OFFSET(6),
  },
#endif /* DMA1_NCHANNELS > 6 */
#if DMA1_NCHANNELS > 7
  {
    .chan     = 7,
    .irq      = LS_IRQ_DMA1CH8,
    .sem      = SEM_INITIALIZER(1),
    .base     = LS_DMA1_BASE + LS_DMACHAN_OFFSET(7),
  },
#endif /* DMA1_NCHANNELS > 7 */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * DMA register access functions
 ****************************************************************************/

/* Get non-channel register from DMA1 or DMA2 */

static inline uint32_t dmabase_getreg(struct ls_dma_s *dmach,
                                      uint32_t offset)
{
  return getreg32(DMA_BASE(dmach->base) + offset);
}

/* Write to non-channel register in DMA1 or DMA2 */

static inline void dmabase_putreg(struct ls_dma_s *dmach,
                                  uint32_t offset, uint32_t value)
{
  putreg32(value, DMA_BASE(dmach->base) + offset);
}

/* Get channel register from DMA1 or DMA2 */

static inline uint32_t dmachan_getreg(struct ls_dma_s *dmach,
                                      uint32_t offset)
{
  return getreg32(dmach->base + offset);
}

/* Write to channel register in DMA1 or DMA2 */

static inline void dmachan_putreg(struct ls_dma_s *dmach,
                                  uint32_t offset, uint32_t value)
{
  putreg32(value, dmach->base + offset);
}

/****************************************************************************
 * Name: ls_dmachandisable
 *
 * Description:
 *  Disable the DMA channel
 *
 ****************************************************************************/

static void ls_dmachandisable(struct ls_dma_s *dmach)
{
  uint32_t regval;

  /* Disable all interrupts at the DMA controller */

  regval = dmachan_getreg(dmach, LS_DMACHAN_CCR_OFFSET);
  regval &= ~DMA_CCR_ALLINTS;

  /* Disable the DMA channel */

  regval &= ~DMA_CCR_EN;
  dmachan_putreg(dmach, LS_DMACHAN_CCR_OFFSET, regval);

  /* Clear pending channel interrupts */

  dmabase_putreg(dmach, LS_DMA_IFCR_OFFSET,
                 DMA_ISR_CHAN_MASK(dmach->chan));
}

/****************************************************************************
 * Name: irq_to_channel_index
 *
 * Description:
 *   Given an IRQ number, find the channel index in the g_dma array.
 *
 * Parameters:
 *   irq: IRQ number as passed to ls_dmainterrupt.
 *
 * Returned Value:
 *   On success (IRQ matches a DMA channel), returns index in the g_dma
 *   array from 0 to DMA_NCHANNELS - 1.  On failure (IRQ does not match
 *   a DMA channel), returns -1.
 *
 ****************************************************************************/

static int irq_to_channel_index(int irq)
{
  int chndx;

  /* Find the DMA channel that matches this IRQ */

  for (chndx = 0; chndx < DMA_NCHANNELS; chndx++)
    {
      if (irq == g_dma[chndx].irq)
        {
          return chndx;
        }
    }

  /* Failed to find the DMA channel for this IRQ */

  return -1;
}

/****************************************************************************
 * Name: ls_dmainterrupt
 *
 * Description:
 *  DMA interrupt handler
 *
 ****************************************************************************/

static int ls_dmainterrupt(int irq, void *context, void *arg)
{
  struct ls_dma_s *dmach;
  uint32_t isr;
  int chndx = 0;

  /* Get the channel structure from the interrupt number */

  chndx = irq_to_channel_index(irq);
  if (chndx < 0)
    {
      DEBUGPANIC();
    }

  dmach = &g_dma[chndx];

  /* Get the interrupt status (for this channel only) */

  isr = dmabase_getreg(dmach, LS_DMA_ISR_OFFSET) &
        DMA_ISR_CHAN_MASK(dmach->chan);

  /* Clear the interrupts we are handling */

  dmabase_putreg(dmach, LS_DMA_IFCR_OFFSET, isr);

  /* Invoke the callback */

  if (dmach->callback)
    {
      dmach->callback(dmach, isr >> DMA_ISR_CHAN_SHIFT(dmach->chan),
                      dmach->arg);
    }

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_dmainitialize
 *
 * Description:
 *   Initialize the DMA subsystem
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void weak_function loongarch_dma_initialize(void)
{
  struct ls_dma_s *dmach;
  int chndx;

  /* Initialize each DMA channel */

  for (chndx = 0; chndx < DMA_NCHANNELS; chndx++)
    {
      dmach = &g_dma[chndx];

      /* Attach DMA interrupt vectors */

      irq_attach(dmach->irq, ls_dmainterrupt, NULL);

      /* Disable the DMA channel */

      ls_dmachandisable(dmach);

      /* Enable the IRQ at the NVIC (still disabled at the DMA controller) */

      up_enable_irq(dmach->irq);
    }
}

/****************************************************************************
 * Name: ls_dmachannel
 *
 * Description:
 *   Allocate a DMA channel.  This function gives the caller mutually
 *   exclusive access to the DMA channel specified by the 'chndx' argument.
 *   DMA channels are shared on the LS:  Devices sharing the same DMA
 *   channel cannot do DMA concurrently!  See the DMACHAN_* definitions in
 *   ls_dma.h.
 *
 *   If the DMA channel is not available, then ls_dmachannel() will wait
 *   until the holder of the channel relinquishes the channel by calling
 *   ls_dmafree().  WARNING: If you have two devices sharing a DMA
 *   channel and the code never releases the channel, the ls_dmachannel
 *   call for the other will hang forever in this function!  Don't let your
 *   design do that!
 *
 *   Hmm.. I suppose this interface could be extended to make a non-blocking
 *   version.  Feel free to do that if that is what you need.
 *
 * Input Parameters:
 *   chndx - Identifies the stream/channel resource.
 *
 * Returned Value:
 *   Provided that 'chndx' is valid, this function ALWAYS returns a non-NULL,
 *   void* DMA channel handle.  (If 'chndx' is invalid, the function will
 *   assert if debug is enabled or do something ignorant otherwise).
 *
 * Assumptions:
 *   - The caller does not hold he DMA channel.
 *   - The caller can wait for the DMA channel to be freed if it is no
 *     available.
 *
 ****************************************************************************/

DMA_HANDLE ls_dmachannel(unsigned int chndef)
{
  int chndx = 0;
  struct ls_dma_s *dmach = NULL;
  int ret;

  chndx = chndef;

  dmach = &g_dma[chndx];

  DEBUGASSERT(chndx < DMA_NCHANNELS);

  /* Get exclusive access to the DMA channel -- OR wait until the channel
   * is available if it is currently being used by another driver
   */

  ret = nxsem_wait_uninterruptible(&dmach->sem);
  if (ret < 0)
    {
      return NULL;
    }

  /* The caller now has exclusive use of the DMA channel */

  return (DMA_HANDLE)dmach;
}

/****************************************************************************
 * Name: ls_dmafree
 *
 * Description:
 *   Release a DMA channel. If another thread is waiting for this DMA channel
 *   in a call to ls_dmachannel, then this function will re-assign the
 *   DMA channel to that thread and wake it up.  NOTE:  The 'handle' used
 *   in this argument must NEVER be used again until ls_dmachannel() is
 *   called again to re-gain access to the channel.
 *
 * Returned Value:
 *   None
 *
 * Assumptions:
 *   - The caller holds the DMA channel.
 *   - There is no DMA in progress
 *
 ****************************************************************************/

void ls_dmafree(DMA_HANDLE handle)
{
  struct ls_dma_s *dmach = (struct ls_dma_s *)handle;

  DEBUGASSERT(handle != NULL);

  /* Release the channel */

  nxsem_post(&dmach->sem);
}

/****************************************************************************
 * Name: ls_dmasetup
 *
 * Description:
 *   Configure DMA before using
 *
 ****************************************************************************/

void ls_dmasetup(DMA_HANDLE handle, uint32_t paddr, uint32_t maddr,
                    size_t ntransfers, uint32_t ccr)
{
  struct ls_dma_s *dmach = (struct ls_dma_s *)handle;
  uint32_t regval;

  /* Then DMA_CNDTRx register can only be modified if the DMA channel is
   * disabled.
   */

  regval  = dmachan_getreg(dmach, LS_DMACHAN_CCR_OFFSET);
  regval &= ~(DMA_CCR_EN);
  dmachan_putreg(dmach, LS_DMACHAN_CCR_OFFSET, regval);

  /* Set the peripheral register address in the DMA_CPARx register. The data
   * will be moved from/to this address to/from the memory after the
   * peripheral event.
   */

  dmachan_putreg(dmach, LS_DMACHAN_CPAR_OFFSET, paddr);

  /* Set the memory address in the DMA_CMARx register. The data will be
   * written to or read from this memory after the peripheral event.
   */

  dmachan_putreg(dmach, LS_DMACHAN_CMAR_OFFSET, maddr);

  /* Configure the total number of data to be transferred in the DMA_CNDTRx
   * register.  After each peripheral event, this value will be decremented.
   */

  dmachan_putreg(dmach, LS_DMACHAN_CNDTR_OFFSET, ntransfers);

  /* Configure the channel priority using the PL[1:0] bits in the DMA_CCRx
   * register.  Configure data transfer direction, circular mode,
   * peripheral & memory incremented mode, peripheral & memory data size,
   * and interrupt after half and/or full transfer in the DMA_CCRx register.
   */

  regval  = dmachan_getreg(dmach, LS_DMACHAN_CCR_OFFSET);
  regval &= ~(DMA_CCR_MEM2MEM | DMA_CCR_PL_MASK | DMA_CCR_MSIZE_MASK |
              DMA_CCR_PSIZE_MASK | DMA_CCR_MINC | DMA_CCR_PINC |
              DMA_CCR_CIRC | DMA_CCR_DIR);
  ccr    &=  (DMA_CCR_MEM2MEM | DMA_CCR_PL_MASK | DMA_CCR_MSIZE_MASK |
              DMA_CCR_PSIZE_MASK | DMA_CCR_MINC | DMA_CCR_PINC |
              DMA_CCR_CIRC | DMA_CCR_DIR);
  regval |= ccr;
  dmachan_putreg(dmach, LS_DMACHAN_CCR_OFFSET, regval);
}

/****************************************************************************
 * Name: ls_dmastart
 *
 * Description:
 *   Start the DMA transfer
 *
 * Assumptions:
 *   - DMA handle allocated by ls_dmachannel()
 *   - No DMA in progress
 *
 ****************************************************************************/

void ls_dmastart(DMA_HANDLE handle, dma_callback_t callback,
                    void *arg, bool half)
{
  struct ls_dma_s *dmach = (struct ls_dma_s *)handle;
  uint32_t ccr;

  DEBUGASSERT(handle != NULL);

  /* Save the callback info.  This will be invoked when the DMA completes. */

  dmach->callback = callback;
  dmach->arg      = arg;

  /* Activate the channel by setting the ENABLE bit in the DMA_CCRx register.
   * As soon as the channel is enabled, it can serve any DMA request from the
   * peripheral connected on the channel.
   */

  ccr  = dmachan_getreg(dmach, LS_DMACHAN_CCR_OFFSET);
  ccr |= DMA_CCR_EN;

  /* In normal mode, interrupt at either half or full completion. In circular
   * mode, always interrupt on buffer wrap, and optionally interrupt at the
   * halfway point.
   */

  if ((ccr & DMA_CCR_CIRC) == 0)
    {
      /* Once half of the bytes are transferred, the half-transfer flag
       * (HTIF) is set and an interrupt is generated if the Half-Transfer
       * Interrupt Enable bit (HTIE) is set. At the end of the transfer, the
       * Transfer Complete Flag (TCIF) is set and an interrupt is generated
       * if the Transfer Complete Interrupt Enable bit (TCIE) is set.
       */

      ccr |= (half ?
              (DMA_CCR_HTIE | DMA_CCR_TEIE) : (DMA_CCR_TCIE | DMA_CCR_TEIE));
    }
  else
    {
      /* In nonstop mode, when the transfer completes it immediately resets
       * and starts again.  The transfer-complete interrupt is thus always
       * enabled, and the half-complete interrupt can be used in circular
       * mode to determine when the buffer is half-full or in double-buffered
       * mode to determine when one of the two buffers is full.
       */

      ccr |= (half ? DMA_CCR_HTIE : 0) | DMA_CCR_TCIE | DMA_CCR_TEIE;
    }

  dmachan_putreg(dmach, LS_DMACHAN_CCR_OFFSET, ccr);
}

/****************************************************************************
 * Name: ls_dmastop
 *
 * Description:
 *   Cancel the DMA.  After ls_dmastop() is called, the DMA channel is
 *   reset and ls_dmasetup() must be called before ls_dmastart() can be
 *   called again
 *
 * Assumptions:
 *   - DMA handle allocated by ls_dmachannel()
 *
 ****************************************************************************/

void ls_dmastop(DMA_HANDLE handle)
{
  struct ls_dma_s *dmach = (struct ls_dma_s *)handle;
  ls_dmachandisable(dmach);
}

/****************************************************************************
 * Name: ls_dmaresidual
 *
 * Description:
 *   Returns the number of bytes remaining to be transferred
 *
 * Assumptions:
 *   - DMA handle allocated by ls_dmachannel()
 *
 ****************************************************************************/

size_t ls_dmaresidual(DMA_HANDLE handle)
{
  struct ls_dma_s *dmach = (struct ls_dma_s *)handle;

  return dmachan_getreg(dmach, LS_DMACHAN_CNDTR_OFFSET);
}

/****************************************************************************
 * Name: ls_dmacapable
 *
 * Description:
 *   Check if the DMA controller can transfer data to/from given memory
 *   address. This depends on the internal connections in the ARM bus matrix
 *   of the processor. Note that this only applies to memory addresses, it
 *   will return false for any peripheral address.
 *
 * Returned Value:
 *   True, if transfer is possible.
 *
 ****************************************************************************/

#ifdef CONFIG_LS_DMACAPABLE
bool ls_dmacapable(uintptr_t maddr, uint32_t count, uint32_t ccr)
{
  uint32_t transfer_size;
  uint32_t mend;

  /* Verify that the address conforms to the memory transfer size.
   * Transfers to/from memory performed by the DMA controller are
   * required to be aligned to their size.
   *
   * See ST RM0090 rev4, section 9.3.11
   *
   * Compute mend inline to avoid a possible non-constant integer
   * multiply.
   */

  switch (ccr & DMA_CCR_MSIZE_MASK)
    {
      case DMA_CCR_MSIZE_8BITS:
        transfer_size = 1;
        mend = maddr + count - 1;
        break;

      case DMA_CCR_MSIZE_16BITS:
        transfer_size = 2;
        mend = maddr + (count << 1) - 1;
        break;

      case DMA_CCR_MSIZE_32BITS:
        transfer_size = 4;
        mend = maddr + (count << 2) - 1;
        break;

      default:
        return false;
    }

  if ((maddr & (transfer_size - 1)) != 0)
    {
      return false;
    }

  /* Verify that the transfer is to a memory region that supports DMA. */

  if ((maddr & LS_REGION_MASK) != (mend & LS_REGION_MASK))
    {
      return false;
    }

  switch (maddr & LS_REGION_MASK)
    {
      case LS_SRAM_BASE:
      case LS_CODE_BASE:

        /* All RAM and flash is supported */

        return true;

      default:

        /* Everything else is unsupported by DMA */

        return false;
    }
}
#endif

/****************************************************************************
 * Name: ls_dmasample
 *
 * Description:
 *   Sample DMA register contents
 *
 * Assumptions:
 *   - DMA handle allocated by ls_dmachannel()
 *
 ****************************************************************************/

#ifdef CONFIG_DEBUG_DMA_INFO
void ls_dmasample(DMA_HANDLE handle, struct ls_dmaregs_s *regs)
{
  struct ls_dma_s *dmach = (struct ls_dma_s *)handle;
  irqstate_t flags;

  flags       = enter_critical_section();
  regs->isr   = dmabase_getreg(dmach, LS_DMA_ISR_OFFSET);
  regs->ccr   = dmachan_getreg(dmach, LS_DMACHAN_CCR_OFFSET);
  regs->cndtr = dmachan_getreg(dmach, LS_DMACHAN_CNDTR_OFFSET);
  regs->cpar  = dmachan_getreg(dmach, LS_DMACHAN_CPAR_OFFSET);
  regs->cmar  = dmachan_getreg(dmach, LS_DMACHAN_CMAR_OFFSET);
  leave_critical_section(flags);
}
#endif

/****************************************************************************
 * Name: ls_dmadump
 *
 * Description:
 *   Dump previously sampled DMA register contents
 *
 * Assumptions:
 *   - DMA handle allocated by ls_dmachannel()
 *
 ****************************************************************************/

#ifdef CONFIG_DEBUG_DMA_INFO
void ls_dmadump(DMA_HANDLE handle, const struct ls_dmaregs_s *regs,
                   const char *msg)
{
  struct ls_dma_s *dmach = (struct ls_dma_s *)handle;
  uintptr_t dmabase = DMA_BASE(dmach->base);

  dmainfo("DMA Registers: %s\n", msg);
  dmainfo("   ISRC[%08" PRIxPTR "]: %08" PRIx32 "\n",
          dmabase + LS_DMA_ISR_OFFSET, regs->isr);
  dmainfo("    CCR[%08" PRIxPTR "]: %08" PRIx32 "\n",
          dmach->base + LS_DMACHAN_CCR_OFFSET, regs->ccr);
  dmainfo("  CNDTR[%08" PRIxPTR "]: %08" PRIx32 "\n",
          dmach->base + LS_DMACHAN_CNDTR_OFFSET, regs->cndtr);
  dmainfo("   CPAR[%08" PRIxPTR "]: %08" PRIx32 "\n",
          dmach->base + LS_DMACHAN_CPAR_OFFSET, regs->cpar);
  dmainfo("   CMAR[%08" PRIxPTR "]: %08" PRIx32 "\n",
          dmach->base + LS_DMACHAN_CMAR_OFFSET, regs->cmar);
}
#endif

#ifdef CONFIG_ARCH_HIPRI_INTERRUPT

/****************************************************************************
 * Name: ls_dma_intack
 *
 * Description:
 *   Public visible interface to acknowledge interrupts on DMA channel
 *
 ****************************************************************************/

void ls_dma_intack(unsigned int chndx, uint32_t isr)
{
  struct ls_dma_s *dmach = &g_dma[chndx];

  dmabase_putreg(dmach, LS_DMA_IFCR_OFFSET, isr);
}

/****************************************************************************
 * Name: ls_dma_intget
 *
 * Description:
 *   Public visible interface to get pending interrupts from DMA channel
 *
 ****************************************************************************/

uint32_t ls_dma_intget(unsigned int chndx)
{
  struct ls_dma_s *dmach = &g_dma[chndx];

  return dmabase_getreg(dmach, LS_DMA_ISR_OFFSET) &
         DMA_ISR_CHAN_MASK(dmach->chan);
}
#endif /* CONFIG_ARCH_HIPRI_INTERRUPT */
