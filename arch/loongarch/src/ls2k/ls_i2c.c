/****************************************************************************
 * arch/loongarch/src/ls2k/ls_i2c.c
 * ref: arch/arm/src/common/stm32/ls_i2c_m3m4_v1.c
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

/* Supports:
 *  - Master operation, 100 kHz (standard) and 400 kHz (full speed)
 *  - Multiple instances (shared bus)
 *  - Interrupt based operation
 *
 * Structure naming:
 *  - Device: structure as defined by the nuttx/i2c/i2c.h
 *  - Instance: represents each individual access to the I2C driver, obtained
 *     by the i2c_init(); it extends the Device structure from the
 *     nuttx/i2c/i2c.h;
 *     Instance points to OPS, to common I2C Hardware private data and
 *     contains its own private data, as frequency, address, mode of
 *     operation (in the future)
 *  - Private: Private data of an I2C Hardware
 *
 * TODO
 *  - Check for all possible deadlocks (as BUSY='1' I2C needs to be reset in
 *    HW using the I2C_CR1_SWRST)
 *  - SMBus support (hardware layer timings are already supported) and add
 *    SMBA gpio pin
 *  - Slave support with multiple addresses (on multiple instances):
 *      - 2 x 7-bit address or
 *      - 1 x 10 bit addresses + 1 x 7 bit address (?)
 *      - plus the broadcast address (general call)
 *  - Multi-master support
 *  - DMA (to get rid of too many CPU wake-ups and interventions)
 *  - Be ready for IPMI
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <nuttx/debug.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <nuttx/clock.h>
#include <nuttx/mutex.h>
#include <nuttx/semaphore.h>
#include <nuttx/i2c/i2c_master.h>

#include <arch/board/board.h>

#include "loongarch_internal.h"
#include "ls_gpio.h"
#include "ls_i2c.h"

/* At least one I2C peripheral must be enabled */

#if defined(CONFIG_LS_I2C0) || defined(CONFIG_LS_I2C1) || \
    defined(CONFIG_LS_I2C2) || defined(CONFIG_LS_I2C3)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if LS_APB_FREQUENCY < 4000000
#  warning "LS_I2C: Periph clk must be at least 4MHz to support 400kHz."
#endif

#if LS_APB_FREQUENCY < 2000000
#  error "LS_I2C: Periph clk must be at least 2MHz to support 100kHz."
#endif

/* Configuration ************************************************************/

/* CONFIG_I2C_POLLED may be set so that I2C interrupts will not be used.
 * Instead, CPU-intensive polling will be used.
 */

/* Interrupt wait timeout in seconds and milliseconds */

#if !defined(CONFIG_LS_I2CTIMEOSEC) && !defined(CONFIG_LS_I2CTIMEOMS)
#  define CONFIG_LS_I2CTIMEOSEC 0
#  define CONFIG_LS_I2CTIMEOMS  500   /* Default is 500 milliseconds */
#elif !defined(CONFIG_LS_I2CTIMEOSEC)
#  define CONFIG_LS_I2CTIMEOSEC 0     /* User provided milliseconds */
#elif !defined(CONFIG_LS_I2CTIMEOMS)
#  define CONFIG_LS_I2CTIMEOMS  0     /* User provided seconds */
#endif

/* Interrupt wait time timeout in system timer ticks */

#ifndef CONFIG_LS_I2CTIMEOTICKS
#  define CONFIG_LS_I2CTIMEOTICKS \
    (SEC2TICK(CONFIG_LS_I2CTIMEOSEC) + MSEC2TICK(CONFIG_LS_I2CTIMEOMS))
#endif

#ifndef CONFIG_LS_I2C_DYNTIMEO_STARTSTOP
#  define CONFIG_LS_I2C_DYNTIMEO_STARTSTOP TICK2USEC(CONFIG_LS_I2CTIMEOTICKS)
#endif

/* Debug ********************************************************************/

/* I2C event trace logic.  NOTE:  trace uses the internal, non-standard,
 * low-level debug interface syslog() but does not require that any other
 * debug is enabled.
 */

#ifndef CONFIG_I2C_TRACE
#  define ls_i2c_tracereset(p)
#  define ls_i2c_tracenew(p,s)
#  define ls_i2c_traceevent(p,e,a)
#  define ls_i2c_tracedump(p)
#endif

#ifndef CONFIG_I2C_NTRACE
#  define CONFIG_I2C_NTRACE 32
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* Interrupt state */

enum ls_intstate_e
{
  INTSTATE_IDLE = 0,      /* No I2C activity */
  INTSTATE_WAITING,       /* Waiting for completion of interrupt activity */
  INTSTATE_DONE,          /* Interrupt activity complete */
};

/* Trace events */

enum ls_trace_e
{
  I2CEVENT_NONE = 0,      /* No events have occurred with this status */
  I2CEVENT_SENDADDR,      /* Start/Master bit set and address sent, param = msgc */
  I2CEVENT_SENDBYTE,      /* Send byte, param = dcnt */
  I2CEVENT_ITBUFEN,       /* Enable buffer interrupts, param = 0 */
  I2CEVENT_RCVBYTE,       /* Read more dta, param = dcnt */
  I2CEVENT_REITBUFEN,     /* Re-enable buffer interrupts, param = 0 */
  I2CEVENT_DISITBUFEN,    /* Disable buffer interrupts, param = 0 */
  I2CEVENT_BTFNOSTART,    /* BTF on last byte with no restart, param = msgc */
  I2CEVENT_BTFRESTART,    /* Last byte sent, re-starting, param = msgc */
  I2CEVENT_BTFSTOP,       /* Last byte sten, send stop, param = 0 */
  I2CEVENT_ERROR          /* Error occurred, param = 0 */
};

/* Trace data */

struct ls_trace_s
{
  uint32_t status;             /* I2C 32-bit SR2|SR1 status */
  uint32_t count;              /* Interrupt count when status change */
  enum ls_intstate_e event;    /* Last event that occurred with this status */
  uint32_t parm;               /* Parameter associated with the event */
  clock_t time;                /* First of event or first status */
};

/* I2C Device hardware configuration */

struct ls_i2c_config_s
{
  uintptr_t base;             /* I2C base address */
  uint32_t clk_bit;           /* Clock enable bit */
  uint32_t reset_bit;         /* Reset bit */
  uint32_t scl_pin;           /* GPIO configuration for SCL as SCL */
  uint32_t sda_pin;           /* GPIO configuration for SDA as SDA */
#ifndef CONFIG_I2C_POLLED
  uint32_t irq;               /* I2C IRQ */
#endif
};

/* I2C Device Private Data */

struct ls_i2c_priv_s
{
  /* Standard I2C operations */

  const struct i2c_ops_s *ops;

  /* Port configuration */

  const struct ls_i2c_config_s *config;

  int refs;                    /* Reference count */
  mutex_t lock;                /* Mutual exclusion lock */
#ifndef CONFIG_I2C_POLLED
  sem_t sem_isr;               /* Interrupt wait semaphore */
#endif
  volatile uint8_t intstate;   /* Interrupt handshake (see enum ls_intstate_e) */

  uint8_t msgc;                /* Message count */
  struct i2c_msg_s *msgv;      /* Message list */
  uint8_t *ptr;                /* Current message buffer */
  uint32_t frequency;          /* Current I2C frequency */
  int dcnt;                    /* Current message length */
  uint16_t flags;              /* Current message flags */

  /* I2C trace support */

#ifdef CONFIG_I2C_TRACE
  int tndx;                    /* Trace array index */
  clock_t start_time;          /* Time when the trace was started */

  /* The actual trace data */

  struct ls_trace_s trace[CONFIG_I2C_NTRACE];
#endif

  uint32_t status;             /* End of transfer SR2|SR1 status */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static inline uint16_t ls_i2c_getreg(struct ls_i2c_priv_s *priv,
                                        uint8_t offset);
static inline void ls_i2c_putreg(struct ls_i2c_priv_s *priv,
                                    uint8_t offset, uint16_t value);
static inline void ls_i2c_modifyreg(struct ls_i2c_priv_s *priv,
                                       uint8_t offset, uint16_t clearbits,
                                       uint16_t setbits);

#ifdef CONFIG_LS_I2C_DYNTIMEO
static uint32_t ls_i2c_toticks(int msgc, struct i2c_msg_s *msgs);
#endif /* CONFIG_LS_I2C_DYNTIMEO */

static inline int  ls_i2c_sem_waitdone(struct ls_i2c_priv_s *priv);
static inline void ls_i2c_sem_waitstop(struct ls_i2c_priv_s *priv);

#ifdef CONFIG_I2C_TRACE
static void ls_i2c_tracereset(struct ls_i2c_priv_s *priv);
static void ls_i2c_tracenew(struct ls_i2c_priv_s *priv,
                               uint32_t status);
static void ls_i2c_traceevent(struct ls_i2c_priv_s *priv,
                                 enum ls_trace_e event, uint32_t parm);
static void ls_i2c_tracedump(struct ls_i2c_priv_s *priv);
#endif /* CONFIG_I2C_TRACE */

static void ls_i2c_setclock(struct ls_i2c_priv_s *priv,
                               uint32_t frequency);
static inline void ls_i2c_sendstart(struct ls_i2c_priv_s *priv);
static inline void ls_i2c_clrstart(struct ls_i2c_priv_s *priv);
static inline void ls_i2c_sendstop(struct ls_i2c_priv_s *priv);
static inline
uint32_t ls_i2c_getstatus(struct ls_i2c_priv_s *priv);

#ifdef I2C1_FSMC_CONFLICT
static inline
uint32_t ls_i2c_disablefsmc(struct ls_i2c_priv_s *priv);
static inline void ls_i2c_enablefsmc(uint32_t ahbenr);
#endif /* I2C1_FSMC_CONFLICT */

static int ls_i2c_isr_process(struct ls_i2c_priv_s *priv);

#ifndef CONFIG_I2C_POLLED
static int ls_i2c_isr(int irq, void *context, void *arg);
#endif /* !CONFIG_I2C_POLLED */

static int ls_i2c_init(struct ls_i2c_priv_s *priv);
static int ls_i2c_deinit(struct ls_i2c_priv_s *priv);
static int ls_i2c_transfer(struct i2c_master_s *dev,
                              struct i2c_msg_s *msgs, int count);
#ifdef CONFIG_I2C_RESET
static int ls_i2c_reset(struct i2c_master_s *dev);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Trace events strings */

#ifdef CONFIG_I2C_TRACE
static const char *g_trace_names[] =
{
  "NONE      ",
  "SENDADDR  ",
  "SENDBYTE  ",
  "ITBUFEN   ",
  "RCVBYTE   ",
  "REITBUFEN ",
  "DISITBUFEN",
  "BTFNOSTART",
  "BTFRESTART",
  "BTFSTOP   ",
  "ERROR     "
};
#endif

/* I2C interface */

static const struct i2c_ops_s ls_i2c_ops =
{
  .transfer = ls_i2c_transfer
#ifdef CONFIG_I2C_RESET
  , .reset  = ls_i2c_reset
#endif
};

/* I2C device structures */

#ifdef CONFIG_LS_I2C0
static const struct ls_i2c_config_s ls_i2c0_config =
{
  .base       = LS_I2C0_BASE,
  .scl_pin    = GPIO_I2C0_SCL,
  .sda_pin    = GPIO_I2C0_SDA,
#ifndef CONFIG_I2C_POLLED
  .irq        = LS_IRQ_I2C0
#endif
};

static struct ls_i2c_priv_s ls_i2c0_priv =
{
  .ops        = &ls_i2c_ops,
  .config     = &ls_i2c0_config,
  .refs       = 0,
  .lock       = NXMUTEX_INITIALIZER,
#ifndef CONFIG_I2C_POLLED
  .sem_isr    = SEM_INITIALIZER(0),
#endif
  .intstate   = INTSTATE_IDLE,
  .msgc       = 0,
  .msgv       = NULL,
  .ptr        = NULL,
  .dcnt       = 0,
  .flags      = 0,
  .status     = 0
};
#endif

#ifdef CONFIG_LS_I2C1
static const struct ls_i2c_config_s ls_i2c1_config =
{
  .base       = LS_I2C1_BASE,
  .scl_pin    = GPIO_I2C1_SCL,
  .sda_pin    = GPIO_I2C1_SDA,
#ifndef CONFIG_I2C_POLLED
  .irq        = LS_IRQ_I2C1
#endif
};

static struct ls_i2c_priv_s ls_i2c1_priv =
{
  .ops        = &ls_i2c_ops,
  .config     = &ls_i2c1_config,
  .refs       = 0,
  .lock       = NXMUTEX_INITIALIZER,
#ifndef CONFIG_I2C_POLLED
  .sem_isr    = SEM_INITIALIZER(0),
#endif
  .intstate   = INTSTATE_IDLE,
  .msgc       = 0,
  .msgv       = NULL,
  .ptr        = NULL,
  .dcnt       = 0,
  .flags      = 0,
  .status     = 0
};
#endif

#ifdef CONFIG_LS_I2C2
static const struct ls_i2c_config_s ls_i2c2_config =
{
  .base       = LS_I2C2_BASE,
  .scl_pin    = GPIO_I2C2_SCL,
  .sda_pin    = GPIO_I2C2_SDA,
#ifndef CONFIG_I2C_POLLED
  .irq        = LS_IRQ_I2C2
#endif
};

static struct ls_i2c_priv_s ls_i2c2_priv =
{
  .ops        = &ls_i2c_ops,
  .config     = &ls_i2c2_config,
  .refs       = 0,
  .lock       = NXMUTEX_INITIALIZER,
#ifndef CONFIG_I2C_POLLED
  .sem_isr    = SEM_INITIALIZER(0),
#endif
  .intstate   = INTSTATE_IDLE,
  .msgc       = 0,
  .msgv       = NULL,
  .ptr        = NULL,
  .dcnt       = 0,
  .flags      = 0,
  .status     = 0
};
#endif

#ifdef CONFIG_LS_I2C3
static const struct ls_i2c_config_s ls_i2c3_config =
{
  .base       = LS_I2C3_BASE,
  .scl_pin    = GPIO_I2C3_SCL,
  .sda_pin    = GPIO_I2C3_SDA,
#ifndef CONFIG_I2C_POLLED
  .irq        = LS_IRQ_I2C3
#endif
};

static struct ls_i2c_priv_s ls_i2c3_priv =
{
  .ops        = &ls_i2c_ops,
  .config     = &ls_i2c3_config,
  .refs       = 0,
  .lock       = NXMUTEX_INITIALIZER,
#ifndef CONFIG_I2C_POLLED
  .sem_isr    = SEM_INITIALIZER(0),
#endif
  .intstate   = INTSTATE_IDLE,
  .msgc       = 0,
  .msgv       = NULL,
  .ptr        = NULL,
  .dcnt       = 0,
  .flags      = 0,
  .status     = 0
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_i2c_getreg
 *
 * Description:
 *   Get a 16-bit register value by offset
 *
 ****************************************************************************/

static inline uint16_t ls_i2c_getreg(struct ls_i2c_priv_s *priv,
                                     uint8_t offset)
{
  return getreg16(priv->config->base + offset);
}

/****************************************************************************
 * Name: ls_i2c_putreg
 *
 * Description:
 *  Put a 16-bit register value by offset
 *
 ****************************************************************************/

static inline void ls_i2c_putreg(struct ls_i2c_priv_s *priv,
                                 uint8_t offset, uint16_t value)
{
  putreg16(value, priv->config->base + offset);
}

/****************************************************************************
 * Name: ls_i2c_modifyreg
 *
 * Description:
 *   Modify a 16-bit register value by offset
 *
 ****************************************************************************/

static inline void ls_i2c_modifyreg(struct ls_i2c_priv_s *priv,
                                    uint8_t offset, uint16_t clearbits,
                                    uint16_t setbits)
{
  modifyreg32(priv->config->base + offset, clearbits, setbits);
}

/****************************************************************************
 * Name: ls_i2c_toticks
 *
 * Description:
 *   Return a micro-second delay based on the number of bytes left to be
 *   processed.
 *
 ****************************************************************************/

#ifdef CONFIG_LS_I2C_DYNTIMEO
static uint32_t ls_i2c_toticks(int msgc, struct i2c_msg_s *msgs)
{
  size_t bytecount = 0;
  int i;

  /* Count the number of bytes left to process */

  for (i = 0; i < msgc; i++)
    {
      bytecount += msgs[i].length;
    }

  /* Then return a number of microseconds based on a user provided scaling
   * factor.
   */

  return USEC2TICK(CONFIG_LS_I2C_DYNTIMEO_USECPERBYTE * bytecount);
}
#endif

/****************************************************************************
 * Name: ls_i2c_sem_waitdone
 *
 * Description:
 *   Wait for a transfer to complete
 *
 ****************************************************************************/

#ifndef CONFIG_I2C_POLLED
static inline int ls_i2c_sem_waitdone(struct ls_i2c_priv_s *priv)
{
  irqstate_t flags;
  uint32_t regval;
  int ret;

  flags = enter_critical_section();

  /* Enable I2C interrupts */

  regval  = ls_i2c_getreg(priv, LS_I2C_CR2_OFFSET);
  regval |= (I2C_CR2_ITERREN | I2C_CR2_ITEVFEN);
  ls_i2c_putreg(priv, LS_I2C_CR2_OFFSET, regval);

  /* Signal the interrupt handler that we are waiting.  NOTE:  Interrupts
   * are currently disabled but will be temporarily re-enabled below when
   * nxsem_tickwait_uninterruptible() sleeps.
   */

  priv->intstate = INTSTATE_WAITING;
  do
    {
      /* Wait until either the transfer is complete or the timeout expires */

#ifdef CONFIG_LS_I2C_DYNTIMEO
      ret = nxsem_tickwait_uninterruptible(&priv->sem_isr,
                            ls_i2c_toticks(priv->msgc, priv->msgv));
#else
      ret = nxsem_tickwait_uninterruptible(&priv->sem_isr,
                                           CONFIG_LS_I2CTIMEOTICKS);
#endif
      if (ret < 0)
        {
          /* Break out of the loop on irrecoverable errors.  This would
           * include timeouts and mystery errors reported by
           * nxsem_tickwait_uninterruptible.
           */

          break;
        }
    }

  /* Loop until the interrupt level transfer is complete. */

  while (priv->intstate != INTSTATE_DONE);

  /* Set the interrupt state back to IDLE */

  priv->intstate = INTSTATE_IDLE;

  /* Disable I2C interrupts */

  regval  = ls_i2c_getreg(priv, LS_I2C_CR2_OFFSET);
  regval &= ~I2C_CR2_ALLINTS;
  ls_i2c_putreg(priv, LS_I2C_CR2_OFFSET, regval);

  leave_critical_section(flags);
  return ret;
}
#else
static inline int ls_i2c_sem_waitdone(struct ls_i2c_priv_s *priv)
{
  clock_t timeout;
  clock_t start;
  clock_t elapsed;
  int ret;

  /* Get the timeout value */

#ifdef CONFIG_LS_I2C_DYNTIMEO
  timeout = ls_i2c_toticks(priv->msgc, priv->msgv);
#else
  timeout = CONFIG_LS_I2CTIMEOTICKS;
#endif

  /* Signal the interrupt handler that we are waiting.  NOTE:  Interrupts
   * are currently disabled but will be temporarily re-enabled below when
   * nxsem_tickwait_uninterruptible() sleeps.
   */

  priv->intstate = INTSTATE_WAITING;
  start = clock_systime_ticks();

  do
    {
      /* Calculate the elapsed time */

      elapsed = clock_systime_ticks() - start;

      /* Poll by simply calling the timer interrupt handler until it
       * reports that it is done.
       */

      ls_i2c_isr_process(priv);
    }

  /* Loop until the transfer is complete. */

  while (priv->intstate != INTSTATE_DONE && elapsed < timeout);

  i2cinfo("intstate: %d elapsed: %ld threshold: %ld status: %08" PRIx32 "\n",
          priv->intstate, (long)elapsed, (long)timeout, priv->status);

  /* Set the interrupt state back to IDLE */

  ret = priv->intstate == INTSTATE_DONE ? OK : -ETIMEDOUT;
  priv->intstate = INTSTATE_IDLE;
  return ret;
}
#endif

/****************************************************************************
 * Name: ls_i2c_sem_waitstop
 *
 * Description:
 *   Wait for a STOP to complete
 *
 ****************************************************************************/

static inline void ls_i2c_sem_waitstop(struct ls_i2c_priv_s *priv)
{
  clock_t start;
  clock_t elapsed;
  clock_t timeout;
  uint32_t cr1;
  uint32_t sr1;

  /* Select a timeout */

#ifdef CONFIG_LS_I2C_DYNTIMEO
  timeout = USEC2TICK(CONFIG_LS_I2C_DYNTIMEO_STARTSTOP);
#else
  timeout = CONFIG_LS_I2CTIMEOTICKS;
#endif

  /* Wait as stop might still be in progress; but stop might also
   * be set because of a timeout error: "The [STOP] bit is set and
   * cleared by software, cleared by hardware when a Stop condition is
   * detected, set by hardware when a timeout error is detected."
   */

  start = clock_systime_ticks();
  do
    {
      /* Calculate the elapsed time */

      elapsed = clock_systime_ticks() - start;

      /* Check for STOP condition */

      cr1 = ls_i2c_getreg(priv, LS_I2C_CR1_OFFSET);
      if ((cr1 & I2C_CR1_STOP) == 0)
        {
          return;
        }
    }

  /* Loop until the stop is complete or a timeout occurs. */

  while (elapsed < timeout);

  /* If we get here then a timeout occurred with the STOP condition
   * still pending.
   */

  i2cinfo("Timeout with CR1: %04" PRIx32 " SR1: %04" PRIx32 "\n", cr1, sr1);
}

/****************************************************************************
 * Name: ls_i2c_trace*
 *
 * Description:
 *   I2C trace instrumentation
 *
 ****************************************************************************/

#ifdef CONFIG_I2C_TRACE
static void ls_i2c_traceclear(struct ls_i2c_priv_s *priv)
{
  struct ls_trace_s *trace = &priv->trace[priv->tndx];

  trace->status = 0;              /* I2C 32-bit SR2|SR1 status */
  trace->count  = 0;              /* Interrupt count when status change */
  trace->event  = I2CEVENT_NONE;  /* Last event that occurred with this status */
  trace->parm   = 0;              /* Parameter associated with the event */
  trace->time   = 0;              /* Time of first status or event */
}

static void ls_i2c_tracereset(struct ls_i2c_priv_s *priv)
{
  /* Reset the trace info for a new data collection */

  priv->tndx       = 0;
  priv->start_time = clock_systime_ticks();
  ls_i2c_traceclear(priv);
}

static void ls_i2c_tracenew(struct ls_i2c_priv_s *priv,
                               uint32_t status)
{
  struct ls_trace_s *trace = &priv->trace[priv->tndx];

  /* Is the current entry uninitialized?  Has the status changed? */

  if (trace->count == 0 || status != trace->status)
    {
      /* Yes.. Was it the status changed?  */

      if (trace->count != 0)
        {
          /* Yes.. bump up the trace index
           * (unless we are out of trace entries)
           */

          if (priv->tndx >= (CONFIG_I2C_NTRACE - 1))
            {
              i2cerr("ERROR: Trace table overflow\n");
              return;
            }

          priv->tndx++;
          trace = &priv->trace[priv->tndx];
        }

      /* Initialize the new trace entry */

      ls_i2c_traceclear(priv);
      trace->status = status;
      trace->count  = 1;
      trace->time   = clock_systime_ticks();
    }
  else
    {
      /* Just increment the count of times that we have seen this status */

      trace->count++;
    }
}

static void ls_i2c_traceevent(struct ls_i2c_priv_s *priv,
                              enum ls_trace_e event, uint32_t parm)
{
  struct ls_trace_s *trace;

  if (event != I2CEVENT_NONE)
    {
      trace = &priv->trace[priv->tndx];

      /* Initialize the new trace entry */

      trace->event = event;
      trace->parm  = parm;

      /* Bump up the trace index (unless we are out of trace entries) */

      if (priv->tndx >= (CONFIG_I2C_NTRACE - 1))
        {
          i2cerr("ERROR: Trace table overflow\n");
          return;
        }

      priv->tndx++;
      ls_i2c_traceclear(priv);
    }
}

static void ls_i2c_tracedump(struct ls_i2c_priv_s *priv)
{
  struct ls_trace_s *trace;
  int i;

  syslog(LOG_DEBUG, "Elapsed time: %ld\n",
         (long)(clock_systime_ticks() - priv->start_time));

  for (i = 0; i < priv->tndx; i++)
    {
      trace = &priv->trace[i];
      syslog(LOG_DEBUG,
         "%2d. STATUS: %08" PRIx32 " COUNT: %3d EVENT: %s(%2d) PARM:"
             " %08" PRIx32 " TIME: %d\n",
             i + 1, trace->status, trace->count, g_trace_names[trace->event],
             trace->event, trace->parm, trace->time - priv->start_time);
    }
}
#endif /* CONFIG_I2C_TRACE */

/****************************************************************************
 * Name: ls_i2c_setclock
 *
 * Description:
 *   Set the I2C clock
 *
 ****************************************************************************/

static void ls_i2c_setclock(struct ls_i2c_priv_s *priv,
                            uint32_t frequency)
{
  uint16_t cr1;
  uint16_t ccr;
  uint16_t trise;
  uint16_t freqmhz;
  uint16_t speed;

  /* Has the I2C bus frequency changed? */

  if (frequency != priv->frequency)
    {
      /* Disable the selected I2C peripheral to configure TRISE */

      cr1 = ls_i2c_getreg(priv, LS_I2C_CR1_OFFSET);
      ls_i2c_putreg(priv, LS_I2C_CR1_OFFSET, cr1 & ~I2C_CR1_PE);

      /* Update timing and control registers */

      freqmhz = (uint16_t)(LS_APB_FREQUENCY / 1000000);
      ccr = 0;

      /* Configure speed in standard mode */

      if (frequency <= 100000)
        {
          /* Standard mode speed calculation */

          speed = (uint16_t)(LS_APB_FREQUENCY / (frequency << 1));

          /* The CCR fault must be >= 4 */

          if (speed < 4)
            {
              /* Set the minimum allowed value */

              speed = 4;
            }

          ccr |= speed;

          /* Set Maximum Rise Time for standard mode */

          trise = freqmhz + 1;
        }

      /* Configure speed in fast mode */

      else /* (frequency <= 400000) */
        {
          /* Fast mode speed calculation with Tlow/Thigh = 16/9 */

#ifdef CONFIG_LS_I2C_DUTY16_9
          speed = (uint16_t)(LS_APB_FREQUENCY / (frequency * 25));

          /* Set DUTY and fast speed bits */

          ccr |= (I2C_CCR_DUTY | I2C_CCR_FS);
#else
          /* Fast mode speed calculation with Tlow/Thigh = 2 */

          speed = (uint16_t)(LS_APB_FREQUENCY / (frequency * 3));

          /* Set fast speed bit */

          ccr |= I2C_CCR_FS;
#endif

          /* Verify that the CCR speed value is nonzero */

          if (speed < 1)
            {
              /* Set the minimum allowed value */

              speed = 1;
            }

          ccr |= speed;

          /* Set Maximum Rise Time for fast mode */

          trise = (uint16_t)(((freqmhz * 300) / 1000) + 1);
        }

      /* Write the new values of the CCR and TRISE registers */

      ls_i2c_putreg(priv, LS_I2C_CCR_OFFSET, ccr);
      ls_i2c_putreg(priv, LS_I2C_TRISE_OFFSET, trise);

      /* Re-enable the peripheral (or not) */

      ls_i2c_putreg(priv, LS_I2C_CR1_OFFSET, cr1);

      /* Save the new I2C frequency */

      priv->frequency = frequency;
    }
}

/****************************************************************************
 * Name: ls_i2c_sendstart
 *
 * Description:
 *   Send the START conditions/force Master mode
 *
 ****************************************************************************/

static inline void ls_i2c_sendstart(struct ls_i2c_priv_s *priv)
{
  /* Disable ACK on receive by default and generate START */

  ls_i2c_modifyreg(priv, LS_I2C_CR1_OFFSET,
                   I2C_CR1_ACK, I2C_CR1_START);
}

/****************************************************************************
 * Name: ls_i2c_clrstart
 *
 * Description:
 *   Clear the STOP, START or PEC condition on certain error recovery steps.
 *
 ****************************************************************************/

static inline void ls_i2c_clrstart(struct ls_i2c_priv_s *priv)
{
  /* "Note: When the STOP, START or PEC bit is set, the software must
   *  not perform any write access to I2C_CR1 before this bit is
   *  cleared by hardware. Otherwise there is a risk of setting a
   *  second STOP, START or PEC request."
   *
   * "The [STOP] bit is set and cleared by software, cleared by hardware
   *  when a Stop condition is detected, set by hardware when a timeout
   *  error is detected.
   *
   * "This [START] bit is set and cleared by software and cleared by hardware
   *  when start is sent or PE=0."  The bit must be cleared by software if
   *  the START is never sent.
   *
   * "This [PEC] bit is set and cleared by software, and cleared by hardware
   *  when PEC is transferred or by a START or Stop condition or when PE=0."
   */

  ls_i2c_modifyreg(priv, LS_I2C_CR1_OFFSET,
                   I2C_CR1_START | I2C_CR1_STOP, 0);
}

/****************************************************************************
 * Name: ls_i2c_sendstop
 *
 * Description:
 *   Send the STOP conditions
 *
 ****************************************************************************/

static inline void ls_i2c_sendstop(struct ls_i2c_priv_s *priv)
{
  ls_i2c_modifyreg(priv, LS_I2C_CR1_OFFSET, I2C_CR1_ACK, I2C_CR1_STOP);
}

/****************************************************************************
 * Name: ls_i2c_getstatus
 *
 * Description:
 *   Get 32-bit status (SR1 and SR2 combined)
 *
 ****************************************************************************/

static inline uint32_t ls_i2c_getstatus(struct ls_i2c_priv_s *priv)
{
  uint32_t status = ls_i2c_getreg(priv, LS_I2C_SR1_OFFSET);
  status |= (ls_i2c_getreg(priv, LS_I2C_SR2_OFFSET) << 16);
  return status;
}

/****************************************************************************
 * Name: ls_i2c_isr_process
 *
 * Description:
 *  Common Interrupt Service Routine
 *
 ****************************************************************************/

static int ls_i2c_isr_process(struct ls_i2c_priv_s *priv)
{
  uint32_t status = ls_i2c_getstatus(priv);

  /* Check for new trace setup */

  ls_i2c_tracenew(priv, status);

  /* Was start bit sent */

  if ((status & I2C_SR1_SB) != 0)
    {
      ls_i2c_traceevent(priv, I2CEVENT_SENDADDR, priv->msgc);

      /* We check for msgc > 0 here as an unexpected interrupt with
       * I2C_SR1_SB set due to noise on the I2C cable can otherwise cause
       * msgc to wrap causing memory overwrite
       */

      if (priv->msgc > 0 && priv->msgv != NULL)
        {
          /* Get run-time data */

          priv->ptr   = priv->msgv->buffer;
          priv->dcnt  = priv->msgv->length;
          priv->flags = priv->msgv->flags;

          /* Send address byte and define addressing mode */

          ls_i2c_putreg(priv, LS_I2C_DR_OFFSET,
                        (priv->flags & I2C_M_TEN) ?
                        0 : ((priv->msgv->addr << 1) |
                        (priv->flags & I2C_M_READ)));

          /* Set ACK for receive mode */

          if (priv->dcnt > 1 && (priv->flags & I2C_M_READ) != 0)
            {
              ls_i2c_modifyreg(priv, LS_I2C_CR1_OFFSET,
                               0, I2C_CR1_ACK);
            }

          /* Increment to next pointer and decrement message count */

          priv->msgv++;
          priv->msgc--;
        }
      else
        {
          /* Clear ISR by writing to DR register */

          ls_i2c_putreg(priv, LS_I2C_DR_OFFSET, 0);
        }
    }

  /* Was address sent, continue with either sending or reading data */

  else if ((priv->flags & I2C_M_READ) == 0 &&
           (status & (I2C_SR1_ADDR | I2C_SR1_TXE)) != 0)
    {
      if (priv->dcnt > 0)
        {
          /* Send a byte */

          ls_i2c_traceevent(priv, I2CEVENT_SENDBYTE, priv->dcnt);
          ls_i2c_putreg(priv, LS_I2C_DR_OFFSET, *priv->ptr++);
          priv->dcnt--;
        }
    }

  else if ((priv->flags & I2C_M_READ) != 0 && (status & I2C_SR1_ADDR) != 0)
    {
      /* Enable RxNE and TxE buffers in order to receive one or multiple
       * bytes
       */

#ifndef CONFIG_I2C_POLLED
      ls_i2c_traceevent(priv, I2CEVENT_ITBUFEN, 0);
      ls_i2c_modifyreg(priv, LS_I2C_CR2_OFFSET, 0, I2C_CR2_ITBUFEN);
#endif
    }

  /* More bytes to read */

  else if ((status & I2C_SR1_RXNE) != 0)
    {
      /* Read a byte, if dcnt goes < 0, then read dummy bytes to ack ISRs */

      if (priv->dcnt > 0)
        {
          ls_i2c_traceevent(priv, I2CEVENT_RCVBYTE, priv->dcnt);

          /* No interrupts or context switches may occur in the following
           * sequence.  Otherwise, additional bytes may be sent by the
           * device.
           */

#ifdef CONFIG_I2C_POLLED
          irqstate_t flags = enter_critical_section();
#endif
          /* Receive a byte */

          *priv->ptr++ = ls_i2c_getreg(priv, LS_I2C_DR_OFFSET);

          /* Disable acknowledge when last byte is to be received */

          priv->dcnt--;
          if (priv->dcnt == 1)
            {
              ls_i2c_modifyreg(priv, LS_I2C_CR1_OFFSET,
                               I2C_CR1_ACK, 0);
            }

#ifdef CONFIG_I2C_POLLED
          leave_critical_section(flags);
#endif
        }
      else
        {
          /* Throw away the unexpected byte */

          ls_i2c_getreg(priv, LS_I2C_DR_OFFSET);
        }
    }
  else if (status & I2C_SR1_TXE)
    {
      /* This should never happen, but it does happen occasionally with lots
       * of noise on the bus. It means the peripheral is expecting more data
       * bytes, but we don't have any to give.
       */

      ls_i2c_putreg(priv, LS_I2C_DR_OFFSET, 0);
    }
  else if (status & I2C_SR1_BTF)
    {
      /* We should have handled all cases where this could happen above, but
       * just to ensure it gets ACKed, lets clear it here
       */

      ls_i2c_getreg(priv, LS_I2C_DR_OFFSET);
    }
  else if (status & I2C_SR1_STOPF)
    {
      /* We should never get this, as we are a master not a slave. Write CR1
       * with its current value to clear the error
       */

      ls_i2c_modifyreg(priv, LS_I2C_CR1_OFFSET, 0, 0);
    }

  /* Do we have more bytes to send, enable/disable buffer interrupts
   * (these ISRs could be replaced by DMAs)
   */

#ifndef CONFIG_I2C_POLLED
  if (priv->dcnt > 0)
    {
      ls_i2c_traceevent(priv, I2CEVENT_REITBUFEN, 0);
      ls_i2c_modifyreg(priv, LS_I2C_CR2_OFFSET, 0, I2C_CR2_ITBUFEN);
    }
  else if (priv->dcnt == 0)
    {
      ls_i2c_traceevent(priv, I2CEVENT_DISITBUFEN, 0);
      ls_i2c_modifyreg(priv, LS_I2C_CR2_OFFSET, I2C_CR2_ITBUFEN, 0);
    }
#endif

  /* Was last byte received or sent?  Hmmm... the F2 and F4 seems to differ
   * from the F1 in that BTF is not set after data is received (only RXNE).
   */

#if defined(CONFIG_LS_LS2K0300F20XX) || defined(CONFIG_LS_LS2K0300F4XXX) || \
    defined(CONFIG_LS_LS2K0300L15XX)
  if (priv->dcnt <= 0 && (status & (I2C_SR1_BTF | I2C_SR1_RXNE)) != 0)
#else
  if (priv->dcnt <= 0 && (status & I2C_SR1_BTF) != 0)
#endif
    {
      ls_i2c_getreg(priv, LS_I2C_DR_OFFSET);    /* ACK ISR */

      /* Do we need to terminate or restart after this byte?
       * If there are more messages to send, then we may:
       *
       *  - continue with repeated start
       *  - or just continue sending writeable part
       *  - or we close down by sending the stop bit
       */

      if (priv->msgc > 0 && priv->msgv != NULL)
        {
          if (priv->msgv->flags & I2C_M_NOSTART)
            {
              ls_i2c_traceevent(priv, I2CEVENT_BTFNOSTART, priv->msgc);
              priv->ptr   = priv->msgv->buffer;
              priv->dcnt  = priv->msgv->length;
              priv->flags = priv->msgv->flags;
              priv->msgv++;
              priv->msgc--;

              /* Restart this ISR! */

#ifndef CONFIG_I2C_POLLED
              ls_i2c_modifyreg(priv, LS_I2C_CR2_OFFSET,
                                  0, I2C_CR2_ITBUFEN);
#endif
            }
          else
            {
              ls_i2c_traceevent(priv, I2CEVENT_BTFRESTART, priv->msgc);
              ls_i2c_sendstart(priv);
            }
        }
      else if (priv->msgv)
        {
          ls_i2c_traceevent(priv, I2CEVENT_BTFSTOP, 0);
          ls_i2c_sendstop(priv);

          /* Is there a thread waiting for this event (there should be) */

#ifndef CONFIG_I2C_POLLED
          if (priv->intstate == INTSTATE_WAITING)
            {
              /* Yes.. inform the thread that the transfer is complete
               * and wake it up.
               */

              nxsem_post(&priv->sem_isr);
              priv->intstate = INTSTATE_DONE;
            }
#else
          priv->intstate = INTSTATE_DONE;
#endif

          /* Mark that we have stopped with this transaction */

          priv->msgv = NULL;
        }
    }

  /* Check for errors, in which case, stop the transfer and return
   * Note that in master reception mode AF becomes set on last byte
   * since ACK is not returned. We should ignore this error.
   */

  if ((status & I2C_SR1_ERRORMASK) != 0)
    {
      ls_i2c_traceevent(priv, I2CEVENT_ERROR, 0);

      /* Clear interrupt flags */

      ls_i2c_putreg(priv, LS_I2C_SR1_OFFSET, 0);

      /* Is there a thread waiting for this event (there should be) */

#ifndef CONFIG_I2C_POLLED
      if (priv->intstate == INTSTATE_WAITING)
        {
          /* Yes.. inform the thread that the transfer is complete
           * and wake it up.
           */

          nxsem_post(&priv->sem_isr);
          priv->intstate = INTSTATE_DONE;
        }
#else
      priv->intstate = INTSTATE_DONE;
#endif
    }

  priv->status = status;
  return OK;
}

/****************************************************************************
 * Name: ls_i2c_isr
 *
 * Description:
 *   Common I2C interrupt service routine
 *
 ****************************************************************************/

#ifndef CONFIG_I2C_POLLED
static int ls_i2c_isr(int irq, void *context, void *arg)
{
  struct ls_i2c_priv_s *priv = (struct ls_i2c_priv_s *)arg;

  DEBUGASSERT(priv != NULL);
  return ls_i2c_isr_process(priv);
}
#endif

/****************************************************************************
 * Name: ls_i2c_init
 *
 * Description:
 *   Setup the I2C hardware, ready for operation with defaults
 *
 ****************************************************************************/

static int ls_i2c_init(struct ls_i2c_priv_s *priv)
{
  /* Configure pins */

  if (ls_configgpio(priv->config->scl_pin) < 0)
    {
      return ERROR;
    }

  if (ls_configgpio(priv->config->sda_pin) < 0)
    {
      ls_unconfiggpio(priv->config->scl_pin);
      return ERROR;
    }

  /* Attach ISRs */

#ifndef CONFIG_I2C_POLLED
  irq_attach(priv->config->irq, ls_i2c_isr, priv);
  up_enable_irq(priv->config->irq);
#endif

  /* Set peripheral frequency, where it must be at least 2 MHz  for 100 kHz
   * or 4 MHz for 400 kHz.  This also disables all I2C interrupts.
   */

  ls_i2c_putreg(priv, LS_I2C_CR2_OFFSET,
               (LS_APB_FREQUENCY / 1000000));

  /* Force a frequency update */

  priv->frequency = 0;

  ls_i2c_setclock(priv, 100000);

  /* Enable I2C */

  ls_i2c_putreg(priv, LS_I2C_CR1_OFFSET, I2C_CR1_PE);
  return OK;
}

/****************************************************************************
 * Name: ls_i2c_deinit
 *
 * Description:
 *   Shutdown the I2C hardware
 *
 ****************************************************************************/

static int ls_i2c_deinit(struct ls_i2c_priv_s *priv)
{
  /* Disable I2C */

  ls_i2c_putreg(priv, LS_I2C_CR1_OFFSET, 0);

  /* Unconfigure GPIO pins */

  ls_unconfiggpio(priv->config->scl_pin);
  ls_unconfiggpio(priv->config->sda_pin);

  /* Disable and detach interrupts */

#ifndef CONFIG_I2C_POLLED
  up_disable_irq(priv->config->irq);
  irq_detach(priv->config->irq);
#endif

  return OK;
}

/****************************************************************************
 * Device Driver Operations
 ****************************************************************************/

/****************************************************************************
 * Name: ls_i2c_transfer
 *
 * Description:
 *   Generic I2C transfer function
 *
 ****************************************************************************/

static int ls_i2c_transfer(struct i2c_master_s *dev,
                              struct i2c_msg_s *msgs, int count)
{
  struct ls_i2c_priv_s *priv = (struct ls_i2c_priv_s *)dev;
  uint32_t status = 0;
#ifdef I2C1_FSMC_CONFLICT
  uint32_t ahbenr;
#endif
  int ret;

  DEBUGASSERT(count > 0);

  /* Ensure that address or flags don't change meanwhile */

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
    {
      return ret;
    }

#ifdef I2C1_FSMC_CONFLICT
  /* Disable FSMC that shares a pin with I2C1 (LBAR) */

  ahbenr = ls_i2c_disablefsmc(priv);

#else
  /* Wait for any STOP in progress.  NOTE:  If we have to disable the FSMC
   * then we cannot do this at the top of the loop, unfortunately.  The STOP
   * will not complete normally if the FSMC is enabled.
   */

  ls_i2c_sem_waitstop(priv);
#endif

  /* Clear any pending error interrupts */

  ls_i2c_putreg(priv, LS_I2C_SR1_OFFSET, 0);

  /* "Note: When the STOP, START or PEC bit is set, the software must
   *  not perform any write access to I2C_CR1 before this bit is
   *  cleared by hardware. Otherwise there is a risk of setting a
   *  second STOP, START or PEC request."  However, if the bits are
   *  not cleared by hardware, then we will have to do that from hardware.
   */

  ls_i2c_clrstart(priv);

  /* Old transfers are done */

  /* Reset ptr and dcnt to ensure an unexpected data interrupt doesn't
   * overwrite stale data.
   */

  priv->dcnt = 0;
  priv->ptr = NULL;

  priv->msgv = msgs;
  priv->msgc = count;

  /* Reset I2C trace logic */

  ls_i2c_tracereset(priv);

  /* Set I2C clock frequency (on change it toggles I2C_CR1_PE !)
   * REVISIT: Note that the frequency is set only on the first message.
   * This could be extended to support different transfer frequencies for
   * each message segment.
   */

  ls_i2c_setclock(priv, msgs->frequency);

  /* Trigger start condition, then the process moves into the ISR.  I2C
   * interrupts will be enabled within ls_i2c_waitdone().
   */

  priv->status = 0;
  ls_i2c_sendstart(priv);

  /* Wait for an ISR, if there was a timeout, fetch latest status to get
   * the BUSY flag.
   */

  if (ls_i2c_sem_waitdone(priv) < 0)
    {
      status = ls_i2c_getstatus(priv);
      ret = -ETIMEDOUT;

      i2cerr("ERROR: Timed out: CR1: 0x%04x status: 0x%08" PRIx32 "\n",
             ls_i2c_getreg(priv, LS_I2C_CR1_OFFSET), status);

      /* "Note: When the STOP, START or PEC bit is set, the software must
       *  not perform any write access to I2C_CR1 before this bit is
       *  cleared by hardware. Otherwise there is a risk of setting a
       *  second STOP, START or PEC request."
       */

      ls_i2c_clrstart(priv);

      /* Clear busy flag in case of timeout */

      status = priv->status & 0xffff;
    }
  else
    {
      /* clear SR2 (BUSY flag) as we've done successfully */

      status = priv->status & 0xffff;
    }

  /* Check for error status conditions */

  if ((status & I2C_SR1_ERRORMASK) != 0)
    {
      /* I2C_SR1_ERRORMASK is the 'OR' of the following individual bits: */

      if (status & I2C_SR1_BERR)
        {
          /* Bus Error */

          ret = -EIO;
        }
      else if (status & I2C_SR1_ARLO)
        {
          /* Arbitration Lost (master mode) */

          ret = -EAGAIN;
        }
      else if (status & I2C_SR1_AF)
        {
          /* Acknowledge Failure */

          ret = -ENXIO;
        }
      else if (status & I2C_SR1_OVR)
        {
          /* Overrun/Underrun */

          ret = -EIO;
        }

      /* This is not an error and should never happen since SMBus is not
       * enabled
       */

      else /* if (status & I2C_SR1_SMBALERT) */
        {
          /* SMBus alert is an optional signal with an interrupt line for
           * devices that want to trade their ability to master for a pin.
           */

          ret = -EINTR;
        }
    }

  /* This is not an error, but should not happen.  The BUSY signal can hang,
   * however, if there are unhealthy devices on the bus that need to be
   * reset.
   * NOTE:  We will only see this busy indication if ls_i2c_sem_waitdone()
   * fails above;  Otherwise it is cleared.
   */

  else if ((status & (I2C_SR2_BUSY << 16)) != 0)
    {
      /* I2C Bus is for some reason busy */

      ret = -EBUSY;
    }

  /* Dump the trace result */

  ls_i2c_tracedump(priv);

#ifdef I2C1_FSMC_CONFLICT
  /* Wait for any STOP in progress.  NOTE:  If we have to disable the FSMC
   * then we cannot do this at the top of the loop, unfortunately.  The STOP
   * will not complete normally if the FSMC is enabled.
   */

  ls_i2c_sem_waitstop(priv);

  /* Re-enable the FSMC */

  ls_i2c_enablefsmc(ahbenr);
#endif

  /* Ensure that any ISR happening after we finish can't overwrite any user
   * data
   */

  priv->dcnt = 0;
  priv->ptr = NULL;

  nxmutex_unlock(&priv->lock);
  return ret;
}

/****************************************************************************
 * Name: ls_i2c_reset
 *
 * Description:
 *   Perform an I2C bus reset in an attempt to break loose stuck I2C devices.
 *
 * Input Parameters:
 *   dev   - Device-specific state data
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

#ifdef CONFIG_I2C_RESET
static int ls_i2c_reset(struct i2c_master_s *dev)
{
  struct ls_i2c_priv_s *priv = (struct ls_i2c_priv_s *)dev;
  unsigned int clock_count;
  unsigned int stretch_count;
  uint32_t scl_gpio;
  uint32_t sda_gpio;
  uint32_t frequency;
  int ret;

  DEBUGASSERT(dev);

  /* Our caller must own a ref */

  DEBUGASSERT(priv->refs > 0);

  /* Lock out other clients */

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
    {
      return ret;
    }

  ret = -EIO;

  /* Save the current frequency */

  frequency = priv->frequency;

  /* De-init the port */

  ls_i2c_deinit(priv);

  /* Use GPIO configuration to un-wedge the bus */

  scl_gpio = MKI2C_OUTPUT(priv->config->scl_pin);
  sda_gpio = MKI2C_OUTPUT(priv->config->sda_pin);

  ls_configgpio(scl_gpio);
  ls_configgpio(sda_gpio);

  /* Let SDA go high */

  ls_gpiowrite(sda_gpio, 1);

  /* Clock the bus until any slaves currently driving it let it go. */

  clock_count = 0;
  while (!ls_gpioread(sda_gpio))
    {
      /* Give up if we have tried too hard */

      if (clock_count++ > 10)
        {
          goto out;
        }

      /* Sniff to make sure that clock stretching has finished.
       *
       * If the bus never relaxes, the reset has failed.
       */

      stretch_count = 0;
      while (!ls_gpioread(scl_gpio))
        {
          /* Give up if we have tried too hard */

          if (stretch_count++ > 10)
            {
              goto out;
            }

          up_udelay(10);
        }

      /* Drive SCL low */

      ls_gpiowrite(scl_gpio, 0);
      up_udelay(10);

      /* Drive SCL high again */

      ls_gpiowrite(scl_gpio, 1);
      up_udelay(10);
    }

  /* Generate a start followed by a stop to reset slave
   * state machines.
   */

  ls_gpiowrite(sda_gpio, 0);
  up_udelay(10);
  ls_gpiowrite(scl_gpio, 0);
  up_udelay(10);
  ls_gpiowrite(scl_gpio, 1);
  up_udelay(10);
  ls_gpiowrite(sda_gpio, 1);
  up_udelay(10);

  /* Revert the GPIO configuration. */

  ls_unconfiggpio(sda_gpio);
  ls_unconfiggpio(scl_gpio);

  /* Re-init the port */

  ls_i2c_init(priv);

  /* Restore the frequency */

  ls_i2c_setclock(priv, frequency);
  ret = OK;

out:

  /* Release the port for reuse by other clients */

  nxmutex_unlock(&priv->lock);
  return ret;
}
#endif /* CONFIG_I2C_RESET */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ls_i2cbus_initialize
 *
 * Description:
 *   Initialize one I2C bus
 *
 ****************************************************************************/

struct i2c_master_s *ls_i2cbus_initialize(int port)
{
  struct ls_i2c_priv_s *priv = NULL;

  /* Get I2C private structure */

  switch (port)
    {
#ifdef CONFIG_LS_I2C0
    case 0:
      priv = (struct ls_i2c_priv_s *)&ls_i2c0_priv;
      break;
#endif
#ifdef CONFIG_LS_I2C1
    case 1:
      priv = (struct ls_i2c_priv_s *)&ls_i2c1_priv;
      break;
#endif
#ifdef CONFIG_LS_I2C2
    case 2:
      priv = (struct ls_i2c_priv_s *)&ls_i2c2_priv;
      break;
#endif
#ifdef CONFIG_LS_I2C3
    case 3:
      priv = (struct ls_i2c_priv_s *)&ls_i2c3_priv;
      break;
#endif
    default:
      return NULL;
    }

  /* Initialize private data for the first time, increment reference count,
   * power-up hardware and configure GPIOs.
   */

  nxmutex_lock(&priv->lock);
  if (priv->refs++ == 0)
    {
      ls_i2c_init(priv);
    }

  nxmutex_unlock(&priv->lock);
  return (struct i2c_master_s *)priv;
}

/****************************************************************************
 * Name: ls_i2cbus_uninitialize
 *
 * Description:
 *   Uninitialize an I2C bus
 *
 ****************************************************************************/

int ls_i2cbus_uninitialize(struct i2c_master_s *dev)
{
  struct ls_i2c_priv_s *priv = (struct ls_i2c_priv_s *)dev;

  DEBUGASSERT(dev);

  /* Decrement reference count and check for underflow */

  if (priv->refs == 0)
    {
      return ERROR;
    }

  nxmutex_lock(&priv->lock);
  if (--priv->refs)
    {
      nxmutex_unlock(&priv->lock);
      return OK;
    }

  /* Disable power and other HW resource (GPIO's) */

  ls_i2c_deinit(priv);
  nxmutex_unlock(&priv->lock);

  return OK;
}

#endif /* CONFIG_LS_I2C0 || CONFIG_LS_I2C1 || CONFIG_LS_I2C2 || CONFIG_LS_I2C3 */
