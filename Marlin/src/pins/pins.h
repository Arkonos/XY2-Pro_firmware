/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * File: pins/pins.h
 *
 * Include pins definitions
 *
 * Pins numbering schemes:
 *
 *  - Digital I/O pin number if used by READ/WRITE macros. (e.g., X_STEP_DIR)
 *    The FastIO headers map digital pins to their ports and functions.
 *
 *  - Analog Input number if used by analogRead or DAC. (e.g., TEMP_n_PIN)
 *    These numbers are the same in any pin mapping.
 */

#define MAX_E_STEPPERS 8

#if NONE(FET_ORDER_EEF, FET_ORDER_EEB, FET_ORDER_EFF, FET_ORDER_EFB, FET_ORDER_SF)
  #if   MB(RAMPS_13_EFB, RAMPS_14_EFB, RAMPS_PLUS_EFB, RAMPS_14_RE_ARM_EFB, RAMPS_SMART_EFB, RAMPS_DUO_EFB, RAMPS4DUE_EFB)
    #define FET_ORDER_EFB 1
  #elif MB(RAMPS_13_EEB, RAMPS_14_EEB, RAMPS_PLUS_EEB, RAMPS_14_RE_ARM_EEB, RAMPS_SMART_EEB, RAMPS_DUO_EEB, RAMPS4DUE_EEB)
    #define FET_ORDER_EEB 1
  #elif MB(RAMPS_13_EFF, RAMPS_14_EFF, RAMPS_PLUS_EFF, RAMPS_14_RE_ARM_EFF, RAMPS_SMART_EFF, RAMPS_DUO_EFF, RAMPS4DUE_EFF)
    #define FET_ORDER_EFF 1
  #elif MB(RAMPS_13_EEF, RAMPS_14_EEF, RAMPS_PLUS_EEF, RAMPS_14_RE_ARM_EEF, RAMPS_SMART_EEF, RAMPS_DUO_EEF, RAMPS4DUE_EEF)
    #define FET_ORDER_EEF 1
  #elif MB(RAMPS_13_SF,  RAMPS_14_SF,  RAMPS_PLUS_SF,  RAMPS_14_RE_ARM_SF,  RAMPS_SMART_SF,  RAMPS_DUO_SF,  RAMPS4DUE_SF)
    #define FET_ORDER_SF 1
  #elif HAS_MULTI_HOTEND || (HAS_EXTRUDERS && HAS_CUTTER)
    #if TEMP_SENSOR_BED
      #define FET_ORDER_EEB 1
    #else
      #define FET_ORDER_EEF 1
    #endif
  #elif TEMP_SENSOR_BED
    #define FET_ORDER_EFB 1
  #else
    #define FET_ORDER_EFF 1
  #endif
#endif

#if !(BOTH(HAS_WIRED_LCD, IS_NEWPANEL) && ANY(PANEL_ONE, VIKI2, miniVIKI, WYH_L12864, MINIPANEL, REPRAPWORLD_KEYPAD))
  #define HAS_FREE_AUX2_PINS 1
#endif

// Test the target within the included pins file
#ifdef __MARLIN_DEPS__
  #define NOT_TARGET(V...) 0
#else
  #define NOT_TARGET(V...) NONE(V)
#endif

#if MB(TRONXY_STM32F103)
  #include "stm32f1/pins_tronxy_stm32f103.h"    // STM32F1  env:tronxy_stm32f103
#endif

//
// Post-process pins according to configured settings
//
#include "pins_postprocess.h"
