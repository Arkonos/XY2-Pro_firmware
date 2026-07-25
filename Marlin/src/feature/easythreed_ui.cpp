/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2021 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

#include "../inc/MarlinConfigPre.h"

#if ENABLED(EASYTHREED_UI)

#include "easythreed_ui.h"
#include "../module/temperature.h"
#include "../gcode/queue.h"
#include "../module/motion.h"
#include "../module/planner.h"
#include "../MarlinCore.h"

EasythreedUI easythreed_ui;

#define BTN_DEBOUNCE_MS 20

void EasythreedUI::init() {
  SET_INPUT_PULLUP(BTN_HOME);     SET_OUTPUT(BTN_HOME_GND);
  SET_INPUT_PULLUP(BTN_FEED);     SET_OUTPUT(BTN_FEED_GND);
  SET_INPUT_PULLUP(BTN_RETRACT);  SET_OUTPUT(BTN_RETRACT_GND);
  SET_INPUT_PULLUP(BTN_PRINT);
  SET_OUTPUT(EASYTHREED_LED_PIN);
}

void EasythreedUI::run() {
  blinkLED();
  loadButton();
  printButton();
}

enum LEDInterval : uint16_t {
  LED_OFF     =    0,
  LED_ON      = 4000,
  LED_BLINK_0 = 2500,
  LED_BLINK_1 = 1500,
  LED_BLINK_2 = 1000,
  LED_BLINK_3 =  800,
  LED_BLINK_4 =  500,
  LED_BLINK_5 =  300,
  LED_BLINK_6 =  150,
  LED_BLINK_7 =   50
};

uint16_t blink_interval_ms = LED_ON;   // Status LED on Start button

void EasythreedUI::blinkLED() {
  static millis_t prev_blink_interval_ms = 0, blink_start_ms = 0;

  if (blink_interval_ms == LED_OFF) { WRITE(EASYTHREED_LED_PIN, HIGH); return; } // OFF
  if (blink_interval_ms >= LED_ON)  { WRITE(EASYTHREED_LED_PIN,  LOW); return; } // ON

  const millis_t ms = millis();
  if (prev_blink_interval_ms != blink_interval_ms) {
    prev_blink_interval_ms = blink_interval_ms;
    blink_start_ms = ms;
  }
  if (PENDING(ms, blink_start_ms + blink_interval_ms))
    WRITE(EASYTHREED_LED_PIN, LOW);
  else if (PENDING(ms, blink_start_ms + 2 * blink_interval_ms))
    WRITE(EASYTHREED_LED_PIN, HIGH);
  else
    blink_start_ms = ms;
}
#if HAS_STEPPER_RESET
  void disableStepperDrivers();
#endif
/
// Print Start/Pause/Resume Button
//
vo
#endif // EASYTHREED_UI
