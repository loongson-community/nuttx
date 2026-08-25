/****************************************************************************
 * arch/loongarch/src/ls2k/ls_spiio.c
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
#include <errno.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/spi/spi.h>
#include <nuttx/kmalloc.h>

#include <arch/board/board.h>

#include "loongarch_internal.h"
#include "hardware/ls_memorymap.h"
#include "hardware/ls_spiio.h"
#include "ls_gpio.h"

#if defined(CONFIG_LS_SPIIO)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LS_SPIIO_CLOCK_FREQ       LS_APB_FREQUENCY
#define SPIIO_TIMEOUT             1000000

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct ls_spiio_priv_s
{
  struct spi_dev_s dev;
  uintptr_t base;
  uint32_t clock_freq;
  uint32_t frequency;
  enum spi_mode_e mode;
  int nbits;
  bool initialized;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int ls_spiio_lock(struct spi_dev_s *dev, bool lock);
static void ls_spiio_select(struct spi_dev_s *dev, uint32_t devid,
                            bool selected);
static uint32_t ls_spiio_setfrequency(struct spi_dev_s *dev,
                                      uint32_t frequency);
static void ls_spiio_setmode(struct spi_dev_s *dev, enum spi_mode_e mode);
static void ls_spiio_setbits(struct spi_dev_s *dev, int nbits);
static uint32_t ls_spiio_send(struct spi_dev_s *dev, uint32_t wd);
#ifdef CONFIG_SPI_EXCHANGE
static void ls_spiio_exchange(struct spi_dev_s *dev,
                              const void *txbuffer, void *rxbuffer,
                              size_t nwords);
#endif
static uint8_t ls_spiio_status(struct spi_dev_s *dev, uint32_t devid);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct spi_ops_s g_spiio_ops =
{
  ls_spiio_lock,
  ls_spiio_select,
  ls_spiio_setfrequency,
#ifdef CONFIG_SPI_DELAY_CONTROL
  NULL,
#endif
  ls_spiio_setmode,
  ls_spiio_setbits,
#ifdef CONFIG_SPI_HWFEATURES
  NULL,
#endif
  ls_spiio_status,
#ifdef CONFIG_SPI_CMDDATA
  NULL,
#endif
  ls_spiio_send,
#ifdef CONFIG_SPI_EXCHANGE
  ls_spiio_exchange,
#else
  NULL,
  NULL,
#endif
#ifdef CONFIG_SPI_TRIGGER
  NULL,
#endif
  NULL
};

static struct ls_spiio_priv_s g_spiio_priv[2] =
{
  {
    .dev =
    {
      .ops = &g_spiio_ops
    },

    .base = PHYS_TO_UNCACHED(LS_SPI2_BASE),
    .clock_freq = LS_SPIIO_CLOCK_FREQ,
    .nbits = 8,
  },

  {
    .dev =
    {
      .ops = &g_spiio_ops
    },

    .base = PHYS_TO_UNCACHED(LS_SPI3_BASE),
    .clock_freq = LS_SPIIO_CLOCK_FREQ,
    .nbits = 8,
  },
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t spiio_read_reg(struct ls_spiio_priv_s *priv, uint32_t offset)
{
  return getreg32(priv->base + offset);
}

static void spiio_write_reg(struct ls_spiio_priv_s *priv, uint32_t offset,
                            uint32_t value)
{
  putreg32(value, priv->base + offset);
}

static uint8_t spiio_read_reg_byte(struct ls_spiio_priv_s *priv,
                                   uint32_t offset)
{
  return getreg8(priv->base + offset);
}

static void spiio_write_reg_byte(struct ls_spiio_priv_s *priv,
                                 uint32_t offset, uint8_t value)
{
  putreg8(value, priv->base + offset);
}

static int spiio_idle_sr1(struct ls_spiio_priv_s *priv, uint32_t bit,
                          uint32_t timeout)
{
  uint32_t sr1;
  uint32_t count = 0;

  while (count < timeout)
    {
      sr1 = spiio_read_reg(priv, LS_SPIIO_SR1);
      if (sr1 & bit)
        {
          return 0;
        }

      count++;
    }

  return -ETIMEDOUT;
}

static int ls_spiio_lock(struct spi_dev_s *dev, bool lock)
{
  return OK;
}

static void ls_spiio_select(struct spi_dev_s *dev, uint32_t devid,
                            bool selected)
{
}

static uint32_t ls_spiio_setfrequency(struct spi_dev_s *dev,
                                      uint32_t frequency)
{
  struct ls_spiio_priv_s *priv = (struct ls_spiio_priv_s *)dev;
  uint32_t cfg2;
  uint32_t brint;
  uint32_t brdec;
  uint32_t div;

  if (frequency == 0)
    {
      frequency = priv->clock_freq / 2;
    }

  div = priv->clock_freq / (2 * frequency);
  if (div < 1)
    {
      div = 1;
    }

  brint = div / 2;
  if (brint < 1)
    {
      brint = 1;
    }

  if (brint > 255)
    {
      brint = 255;
    }

  brdec = div - brint * 2;
  if (brdec > 3)
    {
      brdec = 3;
    }

  cfg2 = spiio_read_reg(priv, LS_SPIIO_CFG2);
  cfg2 &= ~(CFG2_BRINT_MASK | CFG2_BRDEC_MASK);
  cfg2 |= (brint << CFG2_BRINT_SHIFT) | brdec;
  spiio_write_reg(priv, LS_SPIIO_CFG2, cfg2);

  priv->frequency = priv->clock_freq / (2 * div);
  return priv->frequency;
}

static void ls_spiio_setmode(struct spi_dev_s *dev, enum spi_mode_e mode)
{
  struct ls_spiio_priv_s *priv = (struct ls_spiio_priv_s *)dev;
  uint32_t cfg1;

  cfg1 = spiio_read_reg(priv, LS_SPIIO_CFG1);
  cfg1 &= ~(CFG1_CPOL | CFG1_CPHA);

  switch (mode)
    {
      case SPIDEV_MODE0:
        break;
      case SPIDEV_MODE1:
        cfg1 |= CFG1_CPHA;
        break;
      case SPIDEV_MODE2:
        cfg1 |= CFG1_CPOL;
        break;
      case SPIDEV_MODE3:
        cfg1 |= CFG1_CPOL | CFG1_CPHA;
        break;
      default:
        return;
    }

  spiio_write_reg(priv, LS_SPIIO_CFG1, cfg1);
  priv->mode = mode;
}

static void ls_spiio_setbits(struct spi_dev_s *dev, int nbits)
{
  struct ls_spiio_priv_s *priv = (struct ls_spiio_priv_s *)dev;
  uint32_t cfg1;

  if (nbits < 4 || nbits > 32)
    {
      return;
    }

  cfg1 = spiio_read_reg(priv, LS_SPIIO_CFG1);
  cfg1 &= ~CFG1_DSIZE_MASK;
  cfg1 |= ((nbits - 1) << CFG1_DSIZE_SHIFT);
  spiio_write_reg(priv, LS_SPIIO_CFG1, cfg1);

  priv->nbits = nbits;
}

static uint32_t ls_spiio_send(struct spi_dev_s *dev, uint32_t wd)
{
  struct ls_spiio_priv_s *priv = (struct ls_spiio_priv_s *)dev;

  if (spiio_idle_sr1(priv, SR1_TXA, SPIIO_TIMEOUT) < 0)
    {
      return 0;
    }

  spiio_write_reg_byte(priv, LS_SPIIO_DR, (uint8_t)(wd & 0xff));

  spiio_write_reg(priv, LS_SPIIO_CR1,
                  CR1_SPE | CR1_CSTART | CR1_AUTOSUS);

  if (spiio_idle_sr1(priv, SR1_EOT, SPIIO_TIMEOUT) < 0)
    {
      return 0;
    }

  spiio_write_reg(priv, LS_SPIIO_SR1, SR1_EOT);

  if (spiio_idle_sr1(priv, SR1_RXA, SPIIO_TIMEOUT) < 0)
    {
      return 0;
    }

  return (uint32_t)spiio_read_reg_byte(priv, LS_SPIIO_DR);
}

#ifdef CONFIG_SPI_EXCHANGE
static void ls_spiio_exchange(struct spi_dev_s *dev, const void *txbuffer,
                              void *rxbuffer, size_t nwords)
{
  struct ls_spiio_priv_s *priv = (struct ls_spiio_priv_s *)dev;
  const uint8_t *txptr = (const uint8_t *)txbuffer;
  uint8_t *rxptr = (uint8_t *)rxbuffer;
  size_t i;

  for (i = 0; i < nwords; i++)
    {
      if (spiio_idle_sr1(priv, SR1_TXA, SPIIO_TIMEOUT) < 0)
        {
          break;
        }

      if (txptr)
        {
          spiio_write_reg_byte(priv, LS_SPIIO_DR, *txptr++);
        }
      else
        {
          spiio_write_reg_byte(priv, LS_SPIIO_DR, 0x00);
        }

      spiio_write_reg(priv, LS_SPIIO_CR1,
                      CR1_SPE | CR1_CSTART | CR1_AUTOSUS);

      if (spiio_idle_sr1(priv, SR1_EOT, SPIIO_TIMEOUT) < 0)
        {
          break;
        }

      spiio_write_reg(priv, LS_SPIIO_SR1, SR1_EOT);

      if (spiio_idle_sr1(priv, SR1_RXA, SPIIO_TIMEOUT) < 0)
        {
          break;
        }

      if (rxptr)
        {
          *rxptr++ = spiio_read_reg_byte(priv, LS_SPIIO_DR);
        }
      else
        {
          spiio_read_reg_byte(priv, LS_SPIIO_DR);
        }
    }
}
#endif

static uint8_t ls_spiio_status(struct spi_dev_s *dev, uint32_t devid)
{
  return SPI_STATUS_PRESENT;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

struct spi_dev_s *ls_spiio_initialize(int port)
{
  struct ls_spiio_priv_s *priv;
  uint32_t cfg1;
  uint32_t cfg3;

#  ifdef CONFIG_LS_SPIIO0
  if (port == 0)
    {
      /* Configure SPIIO0 pins: CLK, MISO, MOSI and CS */

      ls_configgpio(GPIO_SPIIO0_CLK);
      ls_configgpio(GPIO_SPIIO0_MISO);
      ls_configgpio(GPIO_SPIIO0_MOSI);
      ls_configgpio(GPIO_SPIIO0_CS);
    }
  else
#  endif
#  ifdef CONFIG_LS_SPIIO1
  if (port == 1)
    {
      /* Configure SPIIO1 pins: CLK, MISO, MOSI and CS */

      ls_configgpio(GPIO_SPIIO1_CLK);
      ls_configgpio(GPIO_SPIIO1_MISO);
      ls_configgpio(GPIO_SPIIO1_MOSI);
      ls_configgpio(GPIO_SPIIO1_CS);
    }
  else
#endif
    {
      spierr("ERROR: Unsupported SPI bus: %d\n", port);
      return NULL;
    }

  priv = &g_spiio_priv[port];

  if (!priv->initialized)
    {
      cfg1 = spiio_read_reg(priv, LS_SPIIO_CFG1);
      cfg1 &= ~CFG1_DSIZE_MASK;
      cfg1 |= ((8 - 1) << CFG1_DSIZE_SHIFT);
      spiio_write_reg(priv, LS_SPIIO_CFG1, cfg1);

      cfg3 = spiio_read_reg(priv, LS_SPIIO_CFG3);
      cfg3 |= CFG3_MSTR;
      cfg3 |= CFG3_DIOSWP;
      cfg3 |= CFG3_DIE;
      cfg3 |= CFG3_DOE;
      cfg3 &= ~CFG3_SSMODE_MASK;
      cfg3 |= 0x200;
      spiio_write_reg(priv, LS_SPIIO_CFG3, cfg3);

      spiio_write_reg(priv, LS_SPIIO_IER, 0);

      priv->initialized = true;
    }

  return &priv->dev;
}

#endif
