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
#include "../../MarlinCore.h"
#include "../../module/planner.h"

/**
 * M201: Set max acceleration in units/s^2 for print moves.
 *
 *  X<accel> : Max Acceleration for X
 *  Y<accel> : Max Acceleration for Y
 *  Z<accel> : Max Acceleration for Z
 *       ... : etc
 *  E<accel> : Max Acceleration for Extruder
 *  T<index> : Extruder index to set
 *
 * With XY_FREQUENCY_LIMIT:
 *  F<Hz>      : Frequency limit for XY...IJKUVW
 *  S<percent> : Speed factor percentage.
 */
void GcodeSuite::M201() {
  if (!parser.seen("T" STR_AXES_LOGICAL TERN_(XY_FREQUENCY_LIMIT, "FS")))
    return M201_report();

  const int8_t target_extruder = get_target_extruder_from_command();
  if (target_extruder < 0) return;

  #ifdef XY_FREQUENCY_LIMIT
    if (parser.seenval('F')) planner.set_frequency_limit(parser.value_byte());
    if (parser.seenval('S')) planner.xy_freq_min_speed_factor = constrain(parser.value_float(), 1, 100) / 100;
  #endif

  LOOP_LOGICAL_AXES(i) {
    if (parser.seenval(AXIS_CHAR(i))) {
      const AxisEnum a = (AxisEnum)i;
      planner.set_max_acceleration(a, parser.value_axis_units(a));
    }
  }
}

void GcodeSuite::M201_report(const bool forReplay/*=true*/) {
  #if REPORT_PARAMETER
  report_heading_etc(forReplay, F(STR_MAX_ACCELERATION));
  SERIAL_ECHOLNPGM_P(
    LIST_N(DOUBLE(NUM_AXES),
      PSTR("  M201 X"), LINEAR_UNIT(planner.settings.max_acceleration_mm_per_s2[X_AXIS]),
      SP_Y_STR, LINEAR_UNIT(planner.settings.max_acceleration_mm_per_s2[Y_AXIS]),
      SP_Z_STR, LINEAR_UNIT(planner.settings.max_acceleration_mm_per_s2[Z_AXIS]),
    )
  );
  #endif
}

/**
 * M203: Set maximum feedrate that your machine can sustain (M203 X200 Y200 Z300 E10000) in units/sec
 *
 *       With multiple extruders use T to specify which one.
 */
void GcodeSuite::M203() {
  if (!parser.seen("T" STR_AXES_LOGICAL))
    return M203_report();

  const int8_t target_extruder = get_target_extruder_from_command();
  if (target_extruder < 0) return;

  LOOP_LOGICAL_AXES(i)
    if (parser.seenval(AXIS_CHAR(i))) {
      const AxisEnum a = (AxisEnum)i;
      planner.set_max_feedrate(a, parser.value_axis_units(a));
    }
}

void GcodeSuite::M203_report(const bool forReplay/*=true*/) {
  #if REPORT_PARAMETER
  report_heading_etc(forReplay, F(STR_MAX_FEEDRATES));
  SERIAL_ECHOLNPGM_P(
    LIST_N(DOUBLE(NUM_AXES),
      PSTR("  M203 X"), LINEAR_UNIT(planner.settings.max_feedrate_mm_s[X_AXIS]),
      SP_Y_STR, LINEAR_UNIT(planner.settings.max_feedrate_mm_s[Y_AXIS]),
      SP_Z_STR, LINEAR_UNIT(planner.settings.max_feedrate_mm_s[Z_AXIS]),
    )
  );
  #endif
}

/**
 * M204: Set Accelerations in units/sec^2 (M204 P1200 R3000 T3000)
 *
 *    P = Printing moves
 *    R = Retract only (no X, Y, Z) moves
 *    T = Travel (non printing) moves
 */
void GcodeSuite::M204() {
  if (!parser.seen("PRST"))
    return M204_report();
  else {
    //planner.synchronize();
    // 'S' for legacy compatibility. Should NOT BE USED for new development
    if (parser.seenval('S')) planner.settings.travel_acceleration = planner.settings.acceleration = parser.value_linear_units();
    if (parser.seenval('P')) planner.settings.acceleration = parser.value_linear_units();
    if (parser.seenval('T')) planner.settings.travel_acceleration = parser.value_linear_units();
  }
}

void GcodeSuite::M204_report(const bool forReplay/*=true*/) {
  #if REPORT_PARAMETER
  report_heading_etc(forReplay, F(STR_ACCELERATION_P_R_T));
  SERIAL_ECHOLNPGM_P(
      PSTR("  M204 P"), LINEAR_UNIT(planner.settings.acceleration)
    , SP_T_STR, LINEAR_UNIT(planner.settings.travel_acceleration)
  );
  #endif
}

/**
 * M205: Set Advanced Settings
 *
 *    B = Min Segment Time (µs)
 *    S = Min Feed Rate (units/s)
 *    T = Min Travel Feed Rate (units/s)
 *    X = Max X Jerk (units/sec^2)
 *    Y = Max Y Jerk (units/sec^2)
 *    Z = Max Z Jerk (units/sec^2)
 *    E = Max E Jerk (units/sec^2)
 *    J = Junction Deviation (mm) (If not using CLASSIC_JERK)
 */
void GcodeSuite::M205() {
  if (!parser.seen("BST" TERN_(HAS_JUNCTION_DEVIATION, "J") TERN_(HAS_CLASSIC_JERK, "XYZE")))
    return M205_report();

  //planner.synchronize();
  if (parser.seenval('B')) planner.settings.min_segment_time_us = parser.value_ulong();
  if (parser.seenval('S')) planner.settings.min_feedrate_mm_s = parser.value_linear_units();
  if (parser.seenval('T')) planner.settings.min_travel_feedrate_mm_s = parser.value_linear_units();
  #if HAS_CLASSIC_JERK
    bool seenZ = false;
    LOGICAL_AXIS_CODE(
      if (parser.seenval('X')) planner.set_max_jerk(X_AXIS, parser.value_linear_units()),
      if (parser.seenval('Y')) planner.set_max_jerk(Y_AXIS, parser.value_linear_units()),
      if ((seenZ = parser.seenval('Z'))) planner.set_max_jerk(Z_AXIS, parser.value_linear_units()),
    );
  #endif // HAS_CLASSIC_JERK
}

void GcodeSuite::M205_report(const bool forReplay/*=true*/) {
  #if REPORT_PARAMETER
  report_heading_etc(forReplay, F(
    "Advanced (B<min_segment_time_us> S<min_feedrate> T<min_travel_feedrate>"
    TERN_(HAS_JUNCTION_DEVIATION, " J<junc_dev>")
    #if HAS_CLASSIC_JERK
      NUM_AXIS_GANG(
        " X<max_jerk>", " Y<max_jerk>", " Z<max_jerk>",
        " " STR_I "<max_jerk>", " " STR_J "<max_jerk>", " " STR_K "<max_jerk>",
        " " STR_U "<max_jerk>", " " STR_V "<max_jerk>", " " STR_W "<max_jerk>"
      )
    #endif
    TERN_(HAS_CLASSIC_E_JERK, " E<max_jerk>")
    ")"
  ));
  SERIAL_ECHOLNPGM_P(
      PSTR("  M205 B"), LINEAR_UNIT(planner.settings.min_segment_time_us)
    , PSTR(" S"), LINEAR_UNIT(planner.settings.min_feedrate_mm_s)
    , SP_T_STR, LINEAR_UNIT(planner.settings.min_travel_feedrate_mm_s)
    #if HAS_CLASSIC_JERK
      , LIST_N(DOUBLE(NUM_AXES),
        SP_X_STR, LINEAR_UNIT(planner.max_jerk.x),
        SP_Y_STR, LINEAR_UNIT(planner.max_jerk.y),
        SP_Z_STR, LINEAR_UNIT(planner.max_jerk.z),
      )
    #endif
  );
  #endif
}
