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

//
// Prepare a list of protected pins for M42/M43
//

#if PIN_EXISTS(X_MIN)
  #define _X_MIN X_MIN_PIN,
#else
  #define _X_MIN
#endif
#if PIN_EXISTS(X_MAX)
  #define _X_MAX X_MAX_PIN,
#else
  #define _X_MAX
#endif
#if PIN_EXISTS(X_CS) && AXIS_HAS_SPI(X)
  #define _X_CS X_CS_PIN,
#else
  #define _X_CS
#endif
#if PIN_EXISTS(X_MS1)
  #define _X_MS1 X_MS1_PIN,
#else
  #define _X_MS1
#endif
#if PIN_EXISTS(X_MS2)
  #define _X_MS2 X_MS2_PIN,
#else
  #define _X_MS2
#endif
#if PIN_EXISTS(X_MS3)
  #define _X_MS3 X_MS3_PIN,
#else
  #define _X_MS3
#endif
#if PIN_EXISTS(X_ENABLE)
  #define _X_ENABLE_PIN X_ENABLE_PIN,
#else
  #define _X_ENABLE_PIN
#endif

#define _X_PINS X_STEP_PIN, X_DIR_PIN, _X_ENABLE_PIN _X_MIN _X_MAX _X_MS1 _X_MS2 _X_MS3 _X_CS

#if HAS_Y_AXIS

  #if PIN_EXISTS(Y_MIN)
    #define _Y_MIN Y_MIN_PIN,
  #else
    #define _Y_MIN
  #endif
  #if PIN_EXISTS(Y_MAX)
    #define _Y_MAX Y_MAX_PIN,
  #else
    #define _Y_MAX
  #endif
  #if PIN_EXISTS(Y_CS) && AXIS_HAS_SPI(Y)
    #define _Y_CS Y_CS_PIN,
  #else
    #define _Y_CS
  #endif
  #if PIN_EXISTS(Y_MS1)
    #define _Y_MS1 Y_MS1_PIN,
  #else
    #define _Y_MS1
  #endif
  #if PIN_EXISTS(Y_MS2)
    #define _Y_MS2 Y_MS2_PIN,
  #else
    #define _Y_MS2
  #endif
  #if PIN_EXISTS(Y_MS3)
    #define _Y_MS3 Y_MS3_PIN,
  #else
    #define _Y_MS3
  #endif
  #if PIN_EXISTS(Y_ENABLE)
    #define _Y_ENABLE_PIN Y_ENABLE_PIN,
  #else
    #define _Y_ENABLE_PIN
  #endif

  #define _Y_PINS Y_STEP_PIN, Y_DIR_PIN, _Y_ENABLE_PIN _Y_MIN _Y_MAX _Y_MS1 _Y_MS2 _Y_MS3 _Y_CS

#else

  #define _Y_PINS

#endif

#if HAS_Z_AXIS

  #if PIN_EXISTS(Z_MIN)
    #define _Z_MIN Z_MIN_PIN,
  #else
    #define _Z_MIN
  #endif
  #if PIN_EXISTS(Z_MAX)
    #define _Z_MAX Z_MAX_PIN,
  #else
    #define _Z_MAX
  #endif
  #if PIN_EXISTS(Z_CS) && AXIS_HAS_SPI(Z)
    #define _Z_CS Z_CS_PIN,
  #else
    #define _Z_CS
  #endif
  #if PIN_EXISTS(Z_MS1)
    #define _Z_MS1 Z_MS1_PIN,
  #else
    #define _Z_MS1
  #endif
  #if PIN_EXISTS(Z_MS2)
    #define _Z_MS2 Z_MS2_PIN,
  #else
    #define _Z_MS2
  #endif
  #if PIN_EXISTS(Z_MS3)
    #define _Z_MS3 Z_MS3_PIN,
  #else
    #define _Z_MS3
  #endif
  #if PIN_EXISTS(Z_ENABLE)
    #define _Z_ENABLE_PIN Z_ENABLE_PIN,
  #else
    #define _Z_ENABLE_PIN
  #endif

  #define _Z_PINS Z_STEP_PIN, Z_DIR_PIN, _Z_ENABLE_PIN _Z_MIN _Z_MAX _Z_MS1 _Z_MS2 _Z_MS3 _Z_CS

#else

  #define _Z_PINS

#endif

#define _I_PINS
#define _J_PINS
#define _K_PINS
#define _U_PINS
#define _V_PINS
#define _W_PINS


//
// Extruder Chip Select, Digital Micro-steps
//

// Mixing stepper, Switching stepper, or regular stepper
#define E_NEEDED(N) (ENABLED(MIXING_EXTRUDER) && MIXING_STEPPERS > N) \
                 || (ENABLED(SWITCHING_EXTRUDER) && E_STEPPERS > N) \
                 || (NONE(SWITCHING_EXTRUDER, MIXING_EXTRUDER) && EXTRUDERS > N)

#define _E0_CS
#define _E0_MS1
#define _E0_MS2
#define _E0_MS3

#define _E1_CS
#define _E1_MS1
#define _E1_MS2
#define _E1_MS3

#define _E2_CS
#define _E2_MS1
#define _E2_MS2
#define _E2_MS3

#define _E3_CS
#define _E3_MS1
#define _E3_MS2
#define _E3_MS3

#define _E4_CS
#define _E4_MS1
#define _E4_MS2
#define _E4_MS3

#define _E5_CS
#define _E5_MS1
#define _E5_MS2
#define _E5_MS3

#define _E6_CS
#define _E6_MS1
#define _E6_MS2
#define _E6_MS3

#define _E7_CS
#define _E7_MS1
#define _E7_MS2
#define _E7_MS3

//
// Heaters, Fans, Temp Sensors
//

#define _H0_PINS
#define _H1_PINS
#define _H2_PINS
#define _H3_PINS
#define _H4_PINS
#define _H5_PINS
#define _H6_PINS
#define _H7_PINS

#define DIO_PIN(P) TERN(TARGET_LPC1768, P, analogInputToDigitalPin(P))

//
// Dual X, Dual Y, Multi-Z
// Chip Select and Digital Micro-stepping
//
#define _X2_PINS
#define _Y2_PINS
#define _Z2_PINS
#define _Z3_PINS
#define _Z4_PINS
//
// Generate the final Sensitive Pins array,
// keeping the array as small as possible.
//

#if PIN_EXISTS(PS_ON)
  #define _PS_ON PS_ON_PIN,
#else
  #define _PS_ON
#endif

#if HAS_BED_PROBE && PIN_EXISTS(Z_MIN_PROBE)
  #define _Z_PROBE Z_MIN_PROBE_PIN,
#else
  #define _Z_PROBE
#endif

#if PIN_EXISTS(FAN)
  #define _FAN0 FAN_PIN,
#else
  #define _FAN0
#endif
#if PIN_EXISTS(FAN1)
  #define _FAN1 FAN1_PIN,
#else
  #define _FAN1
#endif
#if PIN_EXISTS(FAN2)
  #define _FAN2 FAN2_PIN,
#else
  #define _FAN2
#endif
#if PIN_EXISTS(FAN3)
  #define _FAN3 FAN3_PIN,
#else
  #define _FAN3
#endif
#if PIN_EXISTS(FAN4)
  #define _FAN4 FAN4_PIN,
#else
  #define _FAN4
#endif
#if PIN_EXISTS(FAN5)
  #define _FAN5 FAN5_PIN,
#else
  #define _FAN5
#endif
#if PIN_EXISTS(FAN6)
  #define _FAN6 FAN6_PIN,
#else
  #define _FAN6
#endif
#if PIN_EXISTS(FAN7)
  #define _FAN7 FAN7_PIN,
#else
  #define _FAN7
#endif
#if PIN_EXISTS(CONTROLLER_FAN)
  #define _FANC CONTROLLER_FAN_PIN,
#else
  #define _FANC
#endif

#if TEMP_SENSOR_BED && PINS_EXIST(TEMP_BED, HEATER_BED)
  #define _BED_PINS HEATER_BED_PIN, DIO_PIN(TEMP_BED_PIN),
#else
  #define _BED_PINS
#endif
#ifndef HAL_SENSITIVE_PINS
  #define HAL_SENSITIVE_PINS
#endif

#ifdef RUNTIME_ONLY_ANALOG_TO_DIGITAL
  #define _SP_END
#else
  #define _SP_END -2

  // Move a regular pin in front to the end
  template<pin_t F, pin_t ...D>
  struct OnlyPins : OnlyPins<D..., F> { };

  // Remove a -1 from the front
  template<pin_t ...D>
  struct OnlyPins<-1, D...> : OnlyPins<D...> { };

  // Remove -2 from the front, emit the rest, cease propagation
  template<pin_t ...D>
  struct OnlyPins<_SP_END, D...> { static constexpr size_t size = sizeof...(D); static constexpr pin_t table[sizeof...(D)] PROGMEM = { D... }; };
#endif

#define SENSITIVE_PINS \
  _X_PINS _Y_PINS _Z_PINS _Z_PROBE \
  _H0_PINS _H1_PINS _H2_PINS _H3_PINS _H4_PINS _H5_PINS _H6_PINS _H7_PINS \
  _PS_ON _FAN0 _FAN1 _FAN2 _FAN3 _FAN4 _FAN5 _FAN6 _FAN7 _FANC \
  _BED_PINS  \
  HAL_SENSITIVE_PINS \
  _SP_END
