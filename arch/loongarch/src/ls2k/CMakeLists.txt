# ##############################################################################
# arch/loongarch/src/ls2k/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

set(SRCS ls_head.S ls_start.c ls_irq.c)

list(APPEND SRCS ls_allocateheap.c ls_clockconfig.c)

list(APPEND SRCS ls_lowputc.c)

if(CONFIG_16550_UART)
  list(APPEND SRCS ls_16550serial.c)
endif()

if(CONFIG_TIMER)
  list(APPEND SRCS ls_timerisr.c)
endif()

if(CONFIG_LS_ADC)
  list(APPEND SRCS ls_adc.c)
endif()

if(CONFIG_LS_DMA)
  list(APPEND SRCS ls_dma.c)
endif()

if(CONFIG_LS_RTC)
  list(APPEND SRCS ls_rtc.c)
endif()

if(CONFIG_LS_I2C)
  list(APPEND SRCS ls_i2c.c)
endif()

if(CONFIG_LS_THERMAL)
  list(APPEND SRCS ls_thermal.c)
endif()

if(CONFIG_LS_TIM)
  list(APPEND SRCS ls_tim.c ls_tim_lowerhalf.c)
endif()

if(CONFIG_LS_TIM_PWM)
  list(APPEND SRCS ls_tim_pwm.c)
endif()

if(CONFIG_LS_PULSECOUNT)
  list(APPEND SRCS ls_pulsecount.c)
endif()

if(CONFIG_LS_CAP)
  list(APPEND SRCS ls_capture.c ls_capture_lowerhalf.c)
endif()

if(CONFIG_LS_QE)
  list(APPEND SRCS ls_qencoder.c)
endif()

if(CONFIG_LS_PWM)
  list(APPEND SRCS ls_pwm.c)
endif()

if(CONFIG_LS_SPI)
  list(APPEND SRCS ls_spi.c)
endif()

if(CONFIG_LS_SPIIO)
  list(APPEND SRCS ls_spiio.c)
endif()

if(CONFIG_LS_WDT)
  list(APPEND SRCS ls_wdt.c)
endif()

if(CONFIG_LS_GPIO)
  list(APPEND SRCS ls_gpio.c)
endif()

target_sources(arch PRIVATE ${SRCS})
