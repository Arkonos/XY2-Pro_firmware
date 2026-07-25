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
#include "../../module/motion.h"

#if ENABLED(I2C_POSITION_ENCODERS)
  #include "../../feature/encoder_i2c.h"
#endif

/**
 * G92: Set the Current Position to the given X [Y [Z [A [B [C [U [V [W ]]]]]]]] [E] values.
 *
 * Behind the scenes the G92 command may modify the Current Position
 * or the Position Shift depending on settings and sub-commands.
 *
 * Since E has no Workspace Offset, it is always set directly.
 *
 * Without Workspace Offsets (e.g., with NO_WORKSPACE_OFFSETS):
 *   G92   : Set NATIVE Current Position to the given X [Y [Z [A [B [C [U [V [W ]]]]]]]] [E].
 *
 * Using Workspace Offsets (default Marlin behavior):
 *   G92   : Modify Workspace Offsets so the reported position shows the given X [Y [Z [A [B [C [U [V [W ]]]]]]]] [E].
 *   G92.1 : Zero XYZ Workspace Offsets (so the reported position = the native position).
 */
void GcodeSuite::G92() {

  bool sync_XYZE = false;

  #if USE_GCODE_SUBCODES
    const uint8_t subcode_G92 = parser.subcode;
  #else
    constexpr uint8_t subcode_G92 = 0;
  #endif

  switch (subcode_G92) {
    default: return;                                                  // Ignore unknown G92.x

    case 0:
      LOOP_LOGICAL_AXES(i) {
        if (parser.seenval(AXIS_CHAR(i))) {
          const float l = parser.value_axis_units((AxisEnum)i),       // Given axis coordinate value, converted to millimeters
                      v = 0 ? l : LOGICAL_TO_NATIVE(l, i),  // Axis position in NATIVE space (applying the existing offset)
                      d = v - current_position[i];                    // How much is the current axis position altered by?
          if (!NEAR_ZERO(d)) {
            #if HAS_POSITION_SHIFT     // When using workspaces...
              position_shift[i] += d;                               // ...most axes offset the workspace...
              update_workspace_offset((AxisEnum)i);
            #else                                                     // Without workspaces...
              if (TERN1(HAS_EXTRUDERS, i != E_AXIS))
                sync_XYZE = true;
              else {
                TERN_(HAS_EXTRUDERS, sync_E = true);
              }
              current_position[i] = v;                                // ...set Current Position directly (like Marlin 1.0)
            #endif
          }
        }
      }
      break;
  }

  if (sync_XYZE) sync_plan_position();

  IF_DISABLED(DIRECT_STEPPING, report_current_position());
}
