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
// Temperature Menu
//

#include "../../inc/MarlinConfig.h"

#if HAS_MARLINUI_MENU && HAS_TEMPERATURE

#include "menu_item.h"
#include "../../module/temperature.h"

//
// "Temperature" submenu items
//

#if HAS_PREHEAT

  void Temperature::lcd_preheat(const uint8_t e, const int8_t indh, const int8_t indb) {
    UNUSED(e); UNUSED(indh); UNUSED(indb);
    #if HAS_HEATED_BED
      if (indb >= 0 && ui.material_preset[indb].bed_temp > 0) setTargetBed(ui.material_preset[indb].bed_temp);
    #endif
    ui.return_to_status();
  }
  #if HAS_HEATED_BED
    inline void _preheat_bed(const uint8_t m) { thermalManager.lcd_preheat(0, -1, m); }
  #endif

  #if HAS_HEATED_BED

    // Set editable.int8 to the Material index before entering this menu
    // because MenuItemBase::itemIndex will be re-used by PREHEAT_ITEMS
    void menu_preheat_m() {
      const uint8_t m = editable.int8; // Don't re-use 'editable' in this menu

      START_MENU();
      BACK_ITEM(MSG_TEMPERATURE);

      #if HAS_HEATED_BED
        ACTION_ITEM_f(ui.get_preheat_label(m), MSG_PREHEAT_M_BEDONLY, []{ _preheat_bed(editable.int8); });
      #endif

      END_MENU();
    }

  #endif // HAS_MULTI_HOTEND || HAS_HEATED_BED

#endif // HAS_PREHEAT

#if HAS_HEATED_BED

  void lcd_cooldown() {
    thermalManager.cooldown();
    ui.return_to_status();
  }

#endif // HAS_HEATED_BED

void menu_temperature() {
  #if HAS_HEATED_BED
    bool has_heat = false;
  #endif

  START_MENU();
  BACK_ITEM(MSG_MAIN);
  // Bed:
  //
  #if HAS_HEATED_BED
    EDIT_ITEM_FAST(int3, MSG_BED, &thermalManager.temp_bed.target, 0, BED_MAX_TARGET, thermalManager.start_watching_bed);
  #endif


  #if HAS_PREHEAT
    //
    // Preheat for all Materials
    //
    LOOP_L_N(m, PREHEAT_COUNT) {
      editable.int8 = m;
      #if HAS_HEATED_BED
        SUBMENU_f(ui.get_preheat_label(m), MSG_PREHEAT_M, menu_preheat_m);
      #endif
    }
  #endif

  #if  HAS_HEATED_BED
    //
    // Cooldown
    //
    if (TERN0(HAS_HEATED_BED, thermalManager.degTargetBed())) has_heat = true;
    if (has_heat) ACTION_ITEM(MSG_COOLDOWN, lcd_cooldown);
  #endif

  END_MENU();
}

#if ENABLED(PREHEAT_SHORTCUT_MENU_ITEM)

  void menu_preheat_only() {
    START_MENU();
    BACK_ITEM(MSG_MAIN);

    LOOP_L_N(m, PREHEAT_COUNT) {
      editable.int8 = m;
      #if HAS_MULTI_HOTEND || HAS_HEATED_BED
        SUBMENU_f(ui.get_preheat_label(m), MSG_PREHEAT_M, menu_preheat_m);
      #else
        ACTION_ITEM_f(ui.get_preheat_label(m), MSG_PREHEAT_M, do_preheat_end_m);
      #endif
    }

    END_MENU();
  }

#endif

#endif // HAS_MARLINUI_MENU && HAS_TEMPERATURE
