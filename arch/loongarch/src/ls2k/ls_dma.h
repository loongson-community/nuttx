/****************************************************************************
 * arch/loongarch/src/ls2k/ls_dma.h
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

#ifndef __ARCH_LOONGARCH_SRC_LS2K_LS_DMA_H
#define __ARCH_LOONGARCH_SRC_LS2K_LS_DMA_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/types.h>

#include "chip.h"

#include "hardware/ls_dma.h"

/* These definitions provide the bit encoding of the 'status' parameter
 * passed to the DMA callback function (see dma_callback_t).
 */

#if defined(HAVE_IP_DMA_V1)
#  define DMA_STATUS_FEIF         0                     /* (Not available in F1) */
#  define DMA_STATUS_DMEIF        0                     /* (Not available in F1) */
#  define DMA_STATUS_TEIF         DMA_CHAN_TEIF_BIT     /* Channel Transfer Error */
#  define DMA_STATUS_HTIF         DMA_CHAN_HTIF_BIT     /* Channel Half Transfer */
#  define DMA_STATUS_TCIF         DMA_CHAN_TCIF_BIT     /* Channel Transfer Complete */
#elif defined(HAVE_IP_DMA_V2)
#  define DMA_STATUS_FEIF         0                     /* Stream FIFO error (ignored) */
#  define DMA_STATUS_DMEIF        DMA_STREAM_DMEIF_BIT  /* Stream direct mode error */
#  define DMA_STATUS_TEIF         DMA_STREAM_TEIF_BIT   /* Stream Transfer Error */
#  define DMA_STATUS_HTIF         DMA_STREAM_HTIF_BIT   /* Stream Half Transfer */
#  define DMA_STATUS_TCIF         DMA_STREAM_TCIF_BIT   /* Stream Transfer Complete */
#endif

#define DMA_STATUS_ERROR          (DMA_STATUS_FEIF | DMA_STATUS_DMEIF | DMA_STATUS_TEIF)
#define DMA_STATUS_SUCCESS        (DMA_STATUS_TCIF | DMA_STATUS_HTIF)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* DMA_HANDLE provides an opaque reference that can be used to represent a
 * DMA channel (F1) or a DMA stream (F4).
 */

typedef void *DMA_HANDLE;

/* Description:
 *   This is the type of the callback that is used to inform the user of the
 *   completion of the DMA.
 *
 * Input Parameters:
 *   handle - Refers to the DMA channel or stream
 *   status - A bit encoded value that provides the completion status.  See
 *            the DMASTATUS_* definitions above.
 *   arg    - A user-provided value that was provided when ls_dmastart()
 *            was called.
 */

typedef void (*dma_callback_t)(DMA_HANDLE handle, uint8_t status, void *arg);

#ifdef CONFIG_DEBUG_DMA_INFO

#if defined(HAVE_IP_DMA_V1)
struct ls_dmaregs_s
{
  uint32_t isr;
  uint32_t ccr;
  uint32_t cndtr;
  uint32_t cpar;
  uint32_t cmar;
};
#elif defined(HAVE_IP_DMA_V2)
struct ls_dmaregs_s
{
  uint32_t lisr;
  uint32_t hisr;
  uint32_t scr;
  uint32_t sndtr;
  uint32_t spar;
  uint32_t sm0ar;
  uint32_t sm1ar;
  uint32_t sfcr;
};
#else
#  error "Unknown LS DMA"
#endif
#endif

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
 * Name: ls_dmachannel
 *
 * Description:
 *   Allocate a DMA channel.  This function gives the caller mutually
 *   exclusive access to the DMA channel specified by the 'chan' argument.
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
 *   chan - Identifies the stream/channel resource
 *
 * Returned Value:
 *   Provided that 'chan' is valid, this function ALWAYS returns a non-NULL,
 *   void* DMA channel handle.  (If 'chan' is invalid, the function will
 *   assert if debug is enabled or do something ignorant otherwise).
 *
 * Assumptions:
 *   - The caller does not hold he DMA channel.
 *   - The caller can wait for the DMA channel to be freed if it is no
 *     available.
 *
 ****************************************************************************/

DMA_HANDLE ls_dmachannel(unsigned int chan);

/****************************************************************************
 * Name: ls_dmafree
 *
 * Description:
 *   Release a DMA channel.  If another thread is waiting for this DMA
 *   channel in a call to ls_dmachannel, then this function will re-
 *   assign the DMA channel to that thread and wake it up.
 *
 *   NOTE:  The 'handle' used in this argument must NEVER be used again
 *   until ls_dmachannel() is called again to re-gain access to the
 *   channel.
 *
 * Returned Value:
 *   None
 *
 * Assumptions:
 *   - The caller holds the DMA channel.
 *   - There is no DMA in progress
 *
 ****************************************************************************/

void ls_dmafree(DMA_HANDLE handle);

/****************************************************************************
 * Name: ls_dmasetup
 *
 * Description:
 *   Configure DMA before using
 *
 ****************************************************************************/

void ls_dmasetup(DMA_HANDLE handle, uint32_t paddr, uint32_t maddr,
                    size_t ntransfers, uint32_t ccr);

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

void ls_dmastart(DMA_HANDLE handle, dma_callback_t callback, void *arg,
                    bool half);

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

void ls_dmastop(DMA_HANDLE handle);

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

size_t ls_dmaresidual(DMA_HANDLE handle);

/****************************************************************************
 * Name: ls_dmacapable
 *
 * Description:
 *   Check if the DMA controller can transfer data to/from given memory
 *   address with the given configuration. This depends on the internal
 *   connections in the ARM bus matrix of the processor. Note that this only
 *   applies to memory addresses, it will return false for any peripheral
 *   address.
 *
 * Returned Value:
 *   True, if transfer is possible.
 *
 ****************************************************************************/

#ifdef CONFIG_LS_DMACAPABLE
bool ls_dmacapable(uintptr_t maddr, uint32_t count, uint32_t ccr);
#else
#  define ls_dmacapable(maddr, count, ccr) (true)
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
void ls_dmasample(DMA_HANDLE handle, struct ls_dmaregs_s *regs);
#else
#  define ls_dmasample(handle,regs)
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
                   const char *msg);
#else
#  define ls_dmadump(handle,regs,msg)
#endif

/* High performance, zero latency DMA interrupts need some additional
 * interfaces.
 *
 * TODO: For now the interface is different for LS2K0300 DMAv1 and
 *       LS2K0300 DMAv2.  It should be unified somehow.
 */

#ifdef CONFIG_ARCH_HIPRI_INTERRUPT

/****************************************************************************
 * Name: ls_dma_intack
 *
 * Description:
 *   Public visible interface to acknowledge interrupts on DMA channel
 *
 ****************************************************************************/

#if defined(HAVE_IP_DMA_V1)
void ls_dma_intack(unsigned int chndx, uint32_t isr);
#elif defined(HAVE_IP_DMA_V2)
void ls_dma_intack(unsigned int controller, uint8_t stream, uint32_t isr);
#endif

/****************************************************************************
 * Name: ls_dma_intget
 *
 * Description:
 *   Public visible interface to get pending interrupts from DMA channel
 *
 ****************************************************************************/

#if defined(HAVE_IP_DMA_V1)
uint32_t ls_dma_intget(unsigned int chndx);
#elif defined(HAVE_IP_DMA_V2)
uint8_t  ls_dma_intget(unsigned int controller, uint8_t stream);
#endif

#endif /* CONFIG_ARCH_HIPRI_INTERRUPT */

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_LOONGARCH_SRC_LS2K_LS_DMA_H */
