/****************************************************************************
 * arch/loongarch/src/ls2k/ls_spi.c
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
#include "hardware/ls_spi.h"
#include "ls_gpio.h"

#if defined(CONFIG_LS_SPI)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LS_SPI_CLOCK_FREQ LS_APB_FREQUENCY

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct ls_spi_priv_s
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

static int ls_spi_lock(struct spi_dev_s *dev, bool lock);
static void ls_spi_select(struct spi_dev_s *dev, uint32_t devid,
                          bool selected);
static uint32_t ls_spi_setfrequency(struct spi_dev_s *dev,
                                    uint32_t frequency);
static void ls_spi_setmode(struct spi_dev_s *dev, enum spi_mode_e mode);
static void ls_spi_setbits(struct spi_dev_s *dev, int nbits);
static uint32_t ls_spi_send(struct spi_dev_s *dev, uint32_t wd);
#ifdef CONFIG_SPI_EXCHANGE
static void ls_spi_exchange(struct spi_dev_s *dev, const void *txbuffer,
                            void *rxbuffer, size_t nwords);
#endif
static uint8_t ls_spi_status(struct spi_dev_s *dev, uint32_t devid);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct spi_ops_s g_spi_ops =
{
  ls_spi_lock,
  ls_spi_select,
  ls_spi_setfrequency,
#ifdef CONFIG_SPI_DELAY_CONTROL
  NULL,
#endif
  ls_spi_setmode,
  ls_spi_setbits,
#ifdef CONFIG_SPI_HWFEATURES
  NULL,
#endif
  ls_spi_status,
#ifdef CONFIG_SPI_CMDDATA
  NULL,
#endif
  ls_spi_send,
#ifdef CONFIG_SPI_EXCHANGE
  ls_spi_exchange,
#else
  NULL,
  NULL,
#endif
#ifdef CONFIG_SPI_TRIGGER
  NULL,
#endif
  NULL
};

static struct ls_spi_priv_s g_spi_priv[2] =
{
  {
    .dev =
    {
      .ops = &g_spi_ops
    },

    .base = PHYS_TO_UNCACHED(LS_SPI0_BASE),
    .clock_freq = LS_SPI_CLOCK_FREQ,
    .nbits = 8,
  },

  {
    .dev =
    {
      .ops = &g_spi_ops
    },

    .base = PHYS_TO_UNCACHED(LS_SPI1_BASE),
    .clock_freq = LS_SPI_CLOCK_FREQ,
    .nbits = 8,
  },
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint8_t spi_read_reg(struct ls_spi_priv_s *priv, uint32_t offset)
{
  return getreg8(priv->base + offset);
}

static void spi_write_reg(struct ls_spi_priv_s *priv, uint32_t offset,
                          uint8_t value)
{
  putreg8(value, priv->base + offset);
}

static int ls_spi_lock(struct spi_dev_s *dev, bool lock)
{
  return OK;
}

static void ls_spi_select(struct spi_dev_s *dev, uint32_t devid,
                          bool selected)
{
}

static uint32_t ls_spi_setfrequency(struct spi_dev_s *dev,
                                    uint32_t frequency)
{
  struct ls_spi_priv_s *priv = (struct ls_spi_priv_s *)dev;
  uint8_t sper_val;
  uint8_t spcr_val;
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

  if (div > 256)
    {
      div = 256;
    }

  sper_val = 0;
  if (div > 1)
    {
      sper_val = (uint8_t)((div - 1) & 0xff);
    }

  spi_write_reg(priv, LS_SPI_SPER, sper_val);

  spcr_val = spi_read_reg(priv, LS_SPI_SPCR);
  spcr_val &= ~0x0c;
  if (div > 256)
    {
      spcr_val |= 0x08;
    }

  spi_write_reg(priv, LS_SPI_SPCR, spcr_val);

  priv->frequency = priv->clock_freq / (2 * div);
  return priv->frequency;
}

static void ls_spi_setmode(struct spi_dev_s *dev, enum spi_mode_e mode)
{
  struct ls_spi_priv_s *priv = (struct ls_spi_priv_s *)dev;
  uint8_t spcr_val;

  spcr_val = spi_read_reg(priv, LS_SPI_SPCR);
  spcr_val &= ~0x0c;

  switch (mode)
    {
      case SPIDEV_MODE0:
        break;
      case SPIDEV_MODE1:
        spcr_val |= 0x04;
        break;
      case SPIDEV_MODE2:
        spcr_val |= 0x08;
        break;
      case SPIDEV_MODE3:
        spcr_val |= 0x0c;
        break;
      default:
        return;
    }

  spi_write_reg(priv, LS_SPI_SPCR, spcr_val);
  priv->mode = mode;
}

static void ls_spi_setbits(struct spi_dev_s *dev, int nbits)
{
  struct ls_spi_priv_s *priv = (struct ls_spi_priv_s *)dev;

  if (nbits != 8)
    {
      return;
    }

  priv->nbits = nbits;
}

static uint32_t ls_spi_send(struct spi_dev_s *dev, uint32_t wd)
{
  struct ls_spi_priv_s *priv = (struct ls_spi_priv_s *)dev;
  uint8_t spsr_val;

  spi_write_reg(priv, LS_SPI_FIFO, (uint8_t)(wd & 0xff));

  do
    {
      spsr_val = spi_read_reg(priv, LS_SPI_SPSR);
    }
  while ((spsr_val & SPSR_SPIF) == 0);

  spi_read_reg(priv, LS_SPI_SPSR);

  return (uint32_t)spi_read_reg(priv, LS_SPI_FIFO);
}

#ifdef CONFIG_SPI_EXCHANGE
static void ls_spi_exchange(struct spi_dev_s *dev, const void *txbuffer,
                            void *rxbuffer, size_t nwords)
{
  struct ls_spi_priv_s *priv = (struct ls_spi_priv_s *)dev;
  const uint8_t *txptr = (const uint8_t *)txbuffer;
  uint8_t *rxptr = (uint8_t *)rxbuffer;
  uint8_t spsr_val;
  uint8_t tx_data;
  uint8_t rx_data;
  size_t i;

  for (i = 0; i < nwords; i++)
    {
      if (txptr)
        {
          tx_data = *txptr++;
        }
      else
        {
          tx_data = 0xff;
        }

      spi_write_reg(priv, LS_SPI_FIFO, tx_data);

      do
        {
          spsr_val = spi_read_reg(priv, LS_SPI_SPSR);
        }
      while ((spsr_val & SPSR_SPIF) == 0);

      spi_read_reg(priv, LS_SPI_SPSR);

      rx_data = spi_read_reg(priv, LS_SPI_FIFO);

      if (rxptr)
        {
          *rxptr++ = rx_data;
        }
    }
}
#endif

static uint8_t ls_spi_status(struct spi_dev_s *dev, uint32_t devid)
{
  return SPI_STATUS_PRESENT;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

struct spi_dev_s *ls_spiflash_initialize(int port)
{
  struct ls_spi_priv_s *priv;
  uint8_t spcr_val;

#ifdef CONFIG_LS_SPI0
  if (port == 0)
    {
      /* Configure SPI0 pins: CLK, MISO, MOSI and CS */

      ls_configgpio(GPIO_SPI0_CLK);
      ls_configgpio(GPIO_SPI0_MISO);
      ls_configgpio(GPIO_SPI0_MOSI);
      ls_configgpio(GPIO_SPI0_CS);
    }
  else
#  endif
#ifdef CONFIG_LS_SPI1
  if (port == 1)
    {
      /* Configure SPI1 pins: CLK, MISO, MOSI and CS */

      ls_configgpio(GPIO_SPI1_CLK);
      ls_configgpio(GPIO_SPI1_MISO);
      ls_configgpio(GPIO_SPI1_MOSI);
      ls_configgpio(GPIO_SPI1_CS);
    }
  else
#endif
    {
      spierr("ERROR: Unsupported SPI bus: %d\n", port);
      return NULL;
    }

  priv = &g_spi_priv[port];

  if (!priv->initialized)
    {
      spi_write_reg(priv, LS_SPI_SPER, 0x07);

      spcr_val = SPCR_SPE;
      spi_write_reg(priv, LS_SPI_SPCR, spcr_val);

      spi_write_reg(priv, LS_SPI_SFCS, 0x00);

      priv->initialized = true;
    }

  return &priv->dev;
}

#endif
