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

//
// Tune Menu
//

#include "../../inc/MarlinConfigPre.h"

#if HAS_MARLINUI_MENU

#include "menu_item.h"
#include "../../module/motion.h"
#include "../../module/planner.h"
#include "../../module/temperature.h"
#include "../../MarlinCore.h"

#if HAS_LEVELING
  #include "../../feature/bedlevel/bedlevel.h"
#endif

#if ENABLED(BABYSTEPPING)

  #include "../../feature/babystep.h"
  #include "../lcdprint.h"
  #if HAS_MARLINUI_U8GLIB
    #include "../dogm/marlinui_DOGM.h"
  #endif

  // TODO: Replace fmsg with MSG_BABYSTEP_N and index substitution

  void _lcd_babystep(const AxisEnum axis, FSTR_P const fmsg) {
    if (ui.use_click()) return ui.goto_previous_screen_no_defer();
    if (ui.encoderPosition) {
      const int16_t steps = int16_t(ui.encoderPosition) * (
        #if ENABLED(BABYSTEP_XY)
          axis == X_AXIS ? BABYSTEP_SIZE_X :
          axis == Y_AXIS ? BABYSTEP_SIZE_Y :
        #endif
        BABYSTEP_SIZE_Z
      );
      ui.encoderPosition = 0;
      ui.refresh(LCDVIEW_REDRAW_NOW);
      babystep.add_steps(axis, steps);
    }
    if (ui.should_draw()) {
      const float mps = planner.mm_per_step[axis];
      MenuEditItemBase::draw_edit_screen(fmsg, BABYSTEP_TO_STR(mps * babystep.accum));
      #if ENABLED(BABYSTEP_DISPLAY_TOTAL)
        const bool in_view = TERN1(HAS_MARLINUI_U8GLIB, PAGE_CONTAINS(LCD_PIXEL_HEIGHT - MENU_FONT_HEIGHT, LCD_PIXEL_HEIGHT - 1));
        if (in_view) {
          TERN_(HAS_MARLINUI_U8GLIB, ui.set_font(FONT_MENU));
          #if ENABLED(TFT_COLOR_UI)
            lcd_moveto(4, 3);
            lcd_put_u8str(GET_TEXT_F(MSG_BABYSTEP_TOTAL));
            lcd_put_lchar(':');
            lcd_moveto(10, 3);
          #else
            lcd_moveto(0, TERN(HAS_MARLINUI_U8GLIB, LCD_PIXEL_HEIGHT - MENU_FONT_DESCENT, LCD_HEIGHT - 1));
            lcd_put_u8str(GET_TEXT_F(MSG_BABYSTEP_TOTAL));
            lcd_put_lchar(':');
          #endif
          lcd_put_u8str(BABYSTEP_TO_STR(mps * babystep.axis_total[BS_TOTAL_IND(axis)]));
        }
      #endif
    }
  }

  inline void _lcd_babystep_go(const screenFunc_t screen) {
    ui.goto_screen(screen);
    ui.defer_status_screen();
    babystep.accum = 0;
  }

  #if ENABLED(BABYSTEP_XY)
    void _lcd_babystep_x() { _lcd_babystep(X_AXIS, GET_TEXT_F(MSG_BABYSTEP_X)); }
    void _lcd_babystep_y() { _lcd_babystep(Y_AXIS, GET_TEXT_F(MSG_BABYSTEP_Y)); }
  #endif

  #if DISABLED(BABYSTEP_ZPROBE_OFFSET)
    void _lcd_babystep_z() { _lcd_babystep(Z_AXIS, GET_TEXT_F(MSG_BABYSTEP_Z)); }
    void lcd_babystep_z()  { _lcd_babystep_go(_lcd_babystep_z); }
  #endif

#endif // BABYSTEPPING

void menu_tune() {
  START_MENU();
  BACK_ITEM(MSG_MAIN);

  //
  // Speed:
  //
  EDIT_ITEM(int3, MSG_SPEED, &feedrate_percentage, 10, 999);

  //
  // Manual bed leveling, Bed Z:
  //
  #if BOTH(MESH_BED_LEVELING, LCD_BED_LEVELING)
    EDIT_ITEM(float43, MSG_BED_Z, &bedlevel.z_offset, -1, 1);
  #endif


  //
  // Bed:
  //
  #if HAS_HEATED_BED
    EDIT_ITEM_FAST(int3, MSG_BED, &thermalManager.temp_bed.target, 0, BED_MAX_TARGET, thermalManager.start_watching_bed);
  #endif

  //
  // Babystep X:
  // Babystep Y:
  // Babystep Z:
  //
  #if ENABLED(BABYSTEPPING)
    #if ENABLED(BABYSTEP_XY)
      SUBMENU_N(X_AXIS, MSG_BABYSTEP_N, []{ _lcd_babystep_go(_lcd_babystep_x); });
      SUBMENU_N(Y_AXIS, MSG_BABYSTEP_N, []{ _lcd_babystep_go(_lcd_babystep_y); });
    #endif
    SUBMENU_N(Z_AXIS, MSG_BABYSTEP_N, lcd_babystep_z);
  #endif

  END_MENU();
}

#endif // HAS_MARLINUI_MENU
