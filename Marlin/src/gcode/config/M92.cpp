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

#include "../gcode.h"
#include "../../module/planner.h"

/**
 * M92: Set axis steps-per-unit for one or more axes, X, Y, Z, [I, [J, [K, [U, [V, [W,]]]]]] and E.
 *      (Follows the same syntax as G92)
 *
 *      With multiple extruders use T to specify which one.
 *
 *      If no argument is given print the current values.
 *
 * With MAGIC_NUMBERS_GCODE:
 *
 *   Use 'H' and/or 'L' to get ideal layer-height information.
 *   H<microsteps> - Specify micro-steps to use. Best guess if not supplied.
 *   L<linear>     - Desired layer height in current units. Nearest good heights are shown.
 */
void GcodeSuite::M92() {

  const int8_t target_extruder = get_target_extruder_from_command();
  if (target_extruder < 0) return;

  // No arguments? Show M92 report.
  if (!parser.seen(STR_AXES_LOGICAL TERN_(MAGIC_NUMBERS_GCODE, "HL")))
    return M92_report(true, target_extruder);

  LOOP_LOGICAL_AXES(i) {
    if (parser.seenval(AXIS_CHAR(i))) {
      if (TERN1(HAS_EXTRUDERS, i != E_AXIS))
        planner.settings.axis_steps_per_mm[i] = parser.value_per_axis_units((AxisEnum)i);
    }
  }
  planner.refresh_positioning();

  #if ENABLED(MAGIC_NUMBERS_GCODE)
    #ifndef Z_MICROSTEPS
      #define Z_MICROSTEPS 16
    #endif
    const float wanted = parser.linearval('L');
    if (parser.seen('H') || wanted) {
      const uint16_t argH = parser.ushortval('H'),
                     micro_steps = argH ?: Z_MICROSTEPS;
      const float z_full_step_mm = micro_steps * planner.mm_per_step[Z_AXIS];
      SERIAL_ECHO_START();
      SERIAL_ECHOPGM("{ micro_steps:", micro_steps, ", z_full_step_mm:", z_full_step_mm);
      if (wanted) {
        const float best = uint16_t(wanted / z_full_step_mm) * z_full_step_mm;
        SERIAL_ECHOPGM(", best:[", best);
        if (best != wanted) { SERIAL_CHAR(','); SERIAL_DECIMAL(best + z_full_step_mm); }
        SERIAL_CHAR(']');
      }
      SERIAL_ECHOLNPGM(" }");
    }
  #endif
}

void GcodeSuite::M92_report(const bool forReplay/*=true*/, const int8_t e/*=-1*/) {
  #if REPORT_PARAMETER
  report_heading_etc(forReplay, F(STR_STEPS_PER_UNIT));
  SERIAL_ECHOPGM_P(LIST_N(DOUBLE(NUM_AXES),
    PSTR("  M92 X"), LINEAR_UNIT(planner.settings.axis_steps_per_mm[X_AXIS]),
    SP_Y_STR, LINEAR_UNIT(planner.settings.axis_steps_per_mm[Y_AXIS]),
    SP_Z_STR, LINEAR_UNIT(planner.settings.axis_steps_per_mm[Z_AXIS]),
  ));
  SERIAL_EOL();
  UNUSED(e);
  #endif
}
