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

#include "../../inc/MarlinConfig.h"

#if ENABLED(BABYSTEPPING)

#include "../gcode.h"
#include "../../feature/babystep.h"
#include "../../module/probe.h"
#include "../../module/planner.h"

#if ENABLED(MESH_BED_LEVELING)
  #include "../../feature/bedlevel/bedlevel.h"
#endif

/**
 * M290: Babystepping
 *
 * Send 'R' or no parameters for a report.
 *
 *  X<linear> - Distance to step X
 *  Y<linear> - Distance to step Y
 *  Z<linear> - Distance to step Z
 *  S<linear> - Distance to step Z (alias for Z)
 */
void GcodeSuite::M290() {
  #if ENABLED(BABYSTEP_XY)
    LOOP_NUM_AXES(a)
      if (parser.seenval(AXIS_CHAR(a)) || (a == Z_AXIS && parser.seenval('S'))) {
        const float offs = constrain(parser.value_axis_units((AxisEnum)a), -2, 2);
        babystep.add_mm((AxisEnum)a, offs);
      }
  #else
    if (parser.seenval('Z') || parser.seenval('S')) {
      const float offs = constrain(parser.value_axis_units(Z_AXIS), -2, 2);
      babystep.add_mm(Z_AXIS, offs);
    }
  #endif

  if (!parser.seen(STR_AXES_MAIN) || parser.seen('R')) {
    SERIAL_ECHO_START();

    #if ENABLED(MESH_BED_LEVELING)
      SERIAL_ECHOLNPGM("MBL Adjust Z", bedlevel.z_offset);
    #endif

    #if ENABLED(BABYSTEP_DISPLAY_TOTAL)
    {
      SERIAL_ECHOLNPGM_P(
        #if ENABLED(BABYSTEP_XY)
            PSTR("Babystep X"), babystep.axis_total[X_AXIS]
          , SP_Y_STR, babystep.axis_total[Y_AXIS]
          , SP_Z_STR
        #else
          PSTR("Babystep Z")
        #endif
        , babystep.axis_total[BS_TOTAL_IND(Z_AXIS)]
      );
    }
    #endif
  }
}

#endif // BABYSTEPPING
