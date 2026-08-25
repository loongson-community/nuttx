/****************************************************************************
 * arch/loongarch/src/ls2k/hardware/ls_pinmap.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_PINMAP_H
#define __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_PINMAP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#if defined(CONFIG_ARCH_CHIP_LS2K0300) || defined(CONFIG_ARCH_CHIP_LS2K0301)
#  include "hardware/ls2k0300_pinmap.h"
#else
#  error "Unsupported LS pin map"
#endif

#endif /* __ARCH_LOONGARCH_SRC_LS2K_HARDWARE_LS_PINMAP_H */
