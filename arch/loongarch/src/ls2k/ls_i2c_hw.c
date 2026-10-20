/****************************************************************************
 * arch/loongarch/src/ls2k/ls_i2c_hw.c
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

/* Polled I2C master transfer for the Loongson LS2K0300.
 *
 * Register timing follows the vendor Linux driver (i2c-ls300.c):
 * 16-bit register accesses only, pclk hardcoded to 10 MHz, 100 kHz
 * standard mode.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <stdint.h>

#include <nuttx/arch.h>

#include "loongarch_internal.h"
#include "ls_i2c.h"
#include "ls_i2c_hw.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Clock init values for pclk = 10 MHz, 100 kHz standard mode */

#define LS_I2C_HW_PCLK_MHZ      10    /* CR2 FREQ field */
#define LS_I2C_HW_CCR           50    /* pclk / (2 * 100kHz), must be >= 4 */
#define LS_I2C_HW_TRISE         11    /* pclk MHz + 1 */

/* Flag polling: 10000 x 10 us = 100 ms per wait */

#define LS_I2C_HW_TIMEOUT_LOOPS 10000
#define LS_I2C_HW_POLL_US       10

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint16_t ls_i2c_hw_mmio_read16(void *ctx, uintptr_t addr)
{
  return getreg16(addr);
}

static void ls_i2c_hw_mmio_write16(void *ctx, uintptr_t addr, uint16_t v)
{
  putreg16(v, addr);
}

static void ls_i2c_hw_mmio_udelay(void *ctx, unsigned us)
{
  up_udelay(us);
}

static uint16_t hw_read(struct ls_i2c_hw_port_s *port, uint16_t offset)
{
  return port->read16(port->ctx, port->base + offset);
}

static void hw_write(struct ls_i2c_hw_port_s *port, uint16_t offset,
                     uint16_t val)
{
  port->write16(port->ctx, port->base + offset, val);
}

static void hw_udelay(struct ls_i2c_hw_port_s *port, unsigned us)
{
  port->udelay(port->ctx, us);
}

/* Timeout recovery: software reset, then re-init the peripheral. */

static void ls_i2c_hw_recover(struct ls_i2c_hw_port_s *port)
{
  hw_write(port, LS_I2C_CR1_OFFSET, I2C_CR1_SWRST);
  hw_udelay(port, 50);
  hw_write(port, LS_I2C_CR1_OFFSET, 0);
  ls_i2c_hw_init(port);
}

/* Wait until any bit of mask is set in SR1. 0 on success,
 * -ETIMEDOUT after 100 ms.
 */

static int ls_i2c_hw_wait_sr1(struct ls_i2c_hw_port_s *port,
                              uint16_t mask)
{
  int i;

  for (i = 0; i < LS_I2C_HW_TIMEOUT_LOOPS; i++)
    {
      if ((hw_read(port, LS_I2C_SR1_OFFSET) & mask) != 0)
        {
          return 0;
        }

      hw_udelay(port, LS_I2C_HW_POLL_US);
    }

  return -ETIMEDOUT;
}

/* Wait for the STOP bit to self-clear (100 ms cap, recovery on
 * timeout).
 */

static int ls_i2c_hw_wait_stop(struct ls_i2c_hw_port_s *port)
{
  int i;

  for (i = 0; i < LS_I2C_HW_TIMEOUT_LOOPS; i++)
    {
      if ((hw_read(port, LS_I2C_CR1_OFFSET) & I2C_CR1_STOP) == 0)
        {
          return 0;
        }

      hw_udelay(port, LS_I2C_HW_POLL_US);
    }

  ls_i2c_hw_recover(port);
  return -ETIMEDOUT;
}

/* Issue STOP and wait for it to clear. */

static int ls_i2c_hw_stop(struct ls_i2c_hw_port_s *port)
{
  hw_write(port, LS_I2C_CR1_OFFSET, I2C_CR1_PE | I2C_CR1_STOP);
  return ls_i2c_hw_wait_stop(port);
}

/* Clear the AF (acknowledge failure) flag, then stop the transfer. */

static int ls_i2c_hw_nak(struct ls_i2c_hw_port_s *port)
{
  uint16_t sr1 = hw_read(port, LS_I2C_SR1_OFFSET);

  hw_write(port, LS_I2C_SR1_OFFSET, sr1 & ~I2C_SR1_AF);
  ls_i2c_hw_stop(port);
  return -ENXIO;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void ls_i2c_hw_bind(struct ls_i2c_hw_port_s *port, uintptr_t base)
{
  port->base    = base;
  port->ctx     = NULL;
  port->read16  = ls_i2c_hw_mmio_read16;
  port->write16 = ls_i2c_hw_mmio_write16;
  port->udelay  = ls_i2c_hw_mmio_udelay;
}

int ls_i2c_hw_init(struct ls_i2c_hw_port_s *port)
{
  uint16_t cr1;

  /* pclk is hardcoded to 10 MHz; deriving it from LS_APB_FREQUENCY is
   * wrong on this chip.
   */

  hw_write(port, LS_I2C_CR2_OFFSET, LS_I2C_HW_PCLK_MHZ);

  /* Disable the peripheral to configure TRISE/CCR */

  cr1 = hw_read(port, LS_I2C_CR1_OFFSET);
  hw_write(port, LS_I2C_CR1_OFFSET, cr1 & ~I2C_CR1_PE);

  /* 100 kHz standard mode */

  hw_write(port, LS_I2C_CCR_OFFSET, LS_I2C_HW_CCR);
  hw_write(port, LS_I2C_TRISE_OFFSET, LS_I2C_HW_TRISE);

  /* Enable peripheral + ACK */

  hw_write(port, LS_I2C_CR1_OFFSET, I2C_CR1_PE | I2C_CR1_ACK);
  return 0;
}

int ls_i2c_hw_xfer(struct ls_i2c_hw_port_s *port, uint8_t addr,
                   int read, uint8_t *buf, uint16_t len)
{
  uint16_t sr1;
  uint16_t cr1;
  uint16_t i;
  int j;
  int ret;

  /* Bus busy? Try one recovery, then give up. */

  if ((hw_read(port, LS_I2C_SR2_OFFSET) & I2C_SR2_BUSY) != 0)
    {
      ls_i2c_hw_recover(port);

      if ((hw_read(port, LS_I2C_SR2_OFFSET) & I2C_SR2_BUSY) != 0)
        {
          return -EBUSY;
        }
    }

  /* START */

  hw_write(port, LS_I2C_CR1_OFFSET,
           I2C_CR1_PE | I2C_CR1_ACK | I2C_CR1_START);

  ret = ls_i2c_hw_wait_sr1(port, I2C_SR1_SB);
  if (ret < 0)
    {
      ls_i2c_hw_recover(port);
      return -ETIMEDOUT;
    }

  /* Address byte */

  hw_write(port, LS_I2C_DR_OFFSET, (uint16_t)((addr << 1) | (read ? 1 : 0)));

  for (j = 0; j < LS_I2C_HW_TIMEOUT_LOOPS; j++)
    {
      sr1 = hw_read(port, LS_I2C_SR1_OFFSET);

      if ((sr1 & I2C_SR1_AF) != 0)
        {
          return ls_i2c_hw_nak(port);   /* slave NAKed the address */
        }

      if ((sr1 & I2C_SR1_ADDR) != 0)
        {
          break;
        }

      hw_udelay(port, LS_I2C_HW_POLL_US);
    }

  if (j >= LS_I2C_HW_TIMEOUT_LOOPS)
    {
      ls_i2c_hw_recover(port);
      return -ETIMEDOUT;
    }

  hw_read(port, LS_I2C_SR2_OFFSET);   /* clear ADDR (read SR1 then SR2) */

  if (!read)
    {
      /* Master transmit */

      for (i = 0; i < len; i++)
        {
          for (j = 0; j < LS_I2C_HW_TIMEOUT_LOOPS; j++)
            {
              sr1 = hw_read(port, LS_I2C_SR1_OFFSET);

              if ((sr1 & I2C_SR1_AF) != 0)
                {
                  return ls_i2c_hw_nak(port);
                }

              if ((sr1 & I2C_SR1_TXE) != 0)
                {
                  break;
                }

              hw_udelay(port, LS_I2C_HW_POLL_US);
            }

          if (j >= LS_I2C_HW_TIMEOUT_LOOPS)
            {
              ls_i2c_hw_recover(port);
              return -ETIMEDOUT;
            }

          hw_write(port, LS_I2C_DR_OFFSET, buf[i]);
        }

      /* All bytes queued: wait for the last one to finish, watch AF */

      for (j = 0; j < LS_I2C_HW_TIMEOUT_LOOPS; j++)
        {
          sr1 = hw_read(port, LS_I2C_SR1_OFFSET);

          if ((sr1 & I2C_SR1_AF) != 0)
            {
              return ls_i2c_hw_nak(port);
            }

          if ((sr1 & I2C_SR1_BTF) != 0)
            {
              break;
            }

          hw_udelay(port, LS_I2C_HW_POLL_US);
        }

      if (j >= LS_I2C_HW_TIMEOUT_LOOPS)
        {
          ls_i2c_hw_recover(port);
          return -ETIMEDOUT;
        }

      ret = ls_i2c_hw_stop(port);
      return ret < 0 ? ret : 0;
    }

  /* Master receive */

  for (i = 0; i < len; i++)
    {
      if (i == len - 1)
        {
          /* Last byte: NAK it and stop right after */

          cr1  = hw_read(port, LS_I2C_CR1_OFFSET);
          cr1 &= ~I2C_CR1_ACK;
          cr1 |= I2C_CR1_STOP;
          hw_write(port, LS_I2C_CR1_OFFSET, cr1);
        }

      ret = ls_i2c_hw_wait_sr1(port, I2C_SR1_RXNE);
      if (ret < 0)
        {
          ls_i2c_hw_recover(port);
          return -ETIMEDOUT;
        }

      buf[i] = (uint8_t)hw_read(port, LS_I2C_DR_OFFSET);
    }

  ret = ls_i2c_hw_wait_stop(port);
  return ret < 0 ? ret : 0;
}
