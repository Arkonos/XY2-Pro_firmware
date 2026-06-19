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
// Advanced Settings Menus
//

#include "../../inc/MarlinConfigPre.h"

#if HAS_MARLINUI_MENU

#include "menu_item.h"
#include "../../MarlinCore.h"
#include "../../module/planner.h"

#if DISABLED(NO_VOLUMETRICS)
  #include "../../gcode/parser.h"
#endif

#if HAS_BED_PROBE
  #include "../../module/probe.h"
#endif

#if ENABLED(SD_FIRMWARE_UPDATE)
  #include "../../module/settings.h"
#endif


void menu_tmc();
void menu_backlash();

#if HAS_MOTOR_CURRENT_DAC

  #include "../../feature/dac/stepper_dac.h"

  void menu_dac() {
    static xyze_uint8_t driverPercent;
    LOOP_LOGICAL_AXES(i) driverPercent[i] = stepper_dac.get_current_percent((AxisEnum)i);
    START_MENU();
    BACK_ITEM(MSG_ADVANCED_SETTINGS);

    LOOP_LOGICAL_AXES(a)
      EDIT_ITEM_N(uint8, a, MSG_DAC_PERCENT_N, &driverPercent[a], 0, 100, []{ stepper_dac.set_current_percents(driverPercent); });

    ACTION_ITEM(MSG_DAC_EEPROM_WRITE, stepper_dac.commit_eeprom);
    END_MENU();
  }

#endif

#if HAS_MOTOR_CURRENT_PWM

  #include "../../module/stepper.h"

  void menu_pwm() {
    START_MENU();
    BACK_ITEM(MSG_ADVANCED_SETTINGS);
    #define EDIT_CURRENT_PWM(LABEL,I) EDIT_ITEM_F(long5, F(LABEL), &stepper.motor_current_setting[I], 100, 2000, stepper.refresh_motor_power)
    #if ANY_PIN(MOTOR_CURRENT_PWM_XY, MOTOR_CURRENT_PWM_X, MOTOR_CURRENT_PWM_Y)
      EDIT_CURRENT_PWM(STR_A STR_B, 0);
    #endif
    #if PIN_EXISTS(MOTOR_CURRENT_PWM_Z)
      EDIT_CURRENT_PWM(STR_C, 1);
    #endif
    #if PIN_EXISTS(MOTOR_CURRENT_PWM_E)
      EDIT_CURRENT_PWM(STR_E, 2);
    #endif
    END_MENU();
  }

#endif

//
// Advanced Settings > Temperature helpers
//

//
// Advanced Settings > Temperature
//
#if SHOW_MENU_ADVANCED_TEMPERATURE

  #if ENABLED(MPC_EDIT_MENU)
    #define MPC_EDIT_DEFS(N) \
      MPC_t &c = thermalManager.temp_hotend[N].constants; \
      TERN_(MPC_INCLUDE_FAN, editable.decimal = c.ambient_xfer_coeff_fan0 + c.fan255_adjustment)
  #endif

  void menu_advanced_temperature() {
    #if ENABLED(MPC_EDIT_MENU) && !HAS_MULTI_HOTEND
      MPC_EDIT_DEFS(0);
    #endif

    START_MENU();
    BACK_ITEM(MSG_ADVANCED_SETTINGS);

    //
    // Autotemp, Min, Max, Fact
    //
    #if BOTH(AUTOTEMP, HAS_TEMP_HOTEND)
      EDIT_ITEM(bool, MSG_AUTOTEMP, &planner.autotemp_enabled);
      EDIT_ITEM(int3, MSG_MIN, &planner.autotemp_min, 0, thermalManager.hotend_max_target(0));
      EDIT_ITEM(int3, MSG_MAX, &planner.autotemp_max, 0, thermalManager.hotend_max_target(0));
      EDIT_ITEM(float42_52, MSG_FACTOR, &planner.autotemp_factor, 0, 10);
    #endif

    //
    // PID-P, PID-I, PID-D, PID-C, PID Autotune
    // PID-P E1, PID-I E1, PID-D E1, PID-C E1, PID Autotune E1
    // PID-P E2, PID-I E2, PID-D E2, PID-C E2, PID Autotune E2
    // PID-P E3, PID-I E3, PID-D E3, PID-C E3, PID Autotune E3
    // PID-P E4, PID-I E4, PID-D E4, PID-C E4, PID Autotune E4
    // PID-P E5, PID-I E5, PID-D E5, PID-C E5, PID Autotune E5
    //

    #define _HOTEND_PID_EDIT_MENU_ITEMS(N) NOOP

    END_MENU();
  }

#endif // SHOW_MENU_ADVANCED_TEMPERATURE

#if DISABLED(SLIM_LCD_MENUS)

  // M203 / M205 Velocity options
  void menu_advanced_velocity() {
    // M203 Max Feedrate
    constexpr xyze_feedrate_t max_fr_edit =
      #ifdef MAX_FEEDRATE_EDIT_VALUES
        MAX_FEEDRATE_EDIT_VALUES
      #elif ENABLED(LIMITED_MAX_FR_EDITING)
        DEFAULT_MAX_FEEDRATE
      #else
        LOGICAL_AXIS_ARRAY_1(9999)
      #endif
    ;
    #if ENABLED(LIMITED_MAX_FR_EDITING) && !defined(MAX_FEEDRATE_EDIT_VALUES)
      const xyze_feedrate_t max_fr_edit_scaled = max_fr_edit * 2;
    #else
      const xyze_feedrate_t &max_fr_edit_scaled = max_fr_edit;
    #endif

    START_MENU();
    BACK_ITEM(MSG_ADVANCED_SETTINGS);

    LOOP_NUM_AXES(a)
      EDIT_ITEM_FAST_N(float5, a, MSG_VMAX_N, &planner.settings.max_feedrate_mm_s[a], 1, max_fr_edit_scaled[a]);

    #if E_STEPPERS
      EDIT_ITEM_FAST_N(float5, E_AXIS, MSG_VMAX_N, &planner.settings.max_feedrate_mm_s[E_AXIS_N(active_extruder)], 1, max_fr_edit_scaled.e);
    #endif
    #if ENABLED(DISTINCT_E_FACTORS)
      LOOP_L_N(n, E_STEPPERS)
        EDIT_ITEM_FAST_N(float5, n, MSG_VMAX_EN, &planner.settings.max_feedrate_mm_s[E_AXIS_N(n)], 1, max_fr_edit_scaled.e);
    #endif

    // M205 S Min Feedrate
    EDIT_ITEM_FAST(float5, MSG_VMIN, &planner.settings.min_feedrate_mm_s, 0, 9999);

    // M205 T Min Travel Feedrate
    EDIT_ITEM_FAST(float5, MSG_VTRAV_MIN, &planner.settings.min_travel_feedrate_mm_s, 0, 9999);

    END_MENU();
  }

  // M201 / M204 Accelerations
  void menu_advanced_acceleration() {
    const float max_accel = _MAX(planner.settings.max_acceleration_mm_per_s2[A_AXIS], planner.settings.max_acceleration_mm_per_s2[B_AXIS], planner.settings.max_acceleration_mm_per_s2[C_AXIS]);

    // M201 settings
    constexpr xyze_ulong_t max_accel_edit =
      #ifdef MAX_ACCEL_EDIT_VALUES
        MAX_ACCEL_EDIT_VALUES
      #elif ENABLED(LIMITED_MAX_ACCEL_EDITING)
        DEFAULT_MAX_ACCELERATION
      #else
        LOGICAL_AXIS_ARRAY_1(99000)
      #endif
    ;
    #if ENABLED(LIMITED_MAX_ACCEL_EDITING) && !defined(MAX_ACCEL_EDIT_VALUES)
      const xyze_ulong_t max_accel_edit_scaled = max_accel_edit * 2;
    #else
      const xyze_ulong_t &max_accel_edit_scaled = max_accel_edit;
    #endif

    START_MENU();
    BACK_ITEM(MSG_ADVANCED_SETTINGS);

    // M204 P Acceleration
    EDIT_ITEM_FAST(float5_25, MSG_ACC, &planner.settings.acceleration, 25, max_accel);

    // M204 T Travel Acceleration
    EDIT_ITEM_FAST(float5_25, MSG_A_TRAVEL, &planner.settings.travel_acceleration, 25, max_accel);

    #define EDIT_AMAX(Q,L) EDIT_ITEM_FAST_N(long5_25, _AXIS(Q), MSG_AMAX_N, &planner.settings.max_acceleration_mm_per_s2[_AXIS(Q)], L, max_accel_edit_scaled[_AXIS(Q)], []{ planner.refresh_acceleration_rates(); })
    NUM_AXIS_CODE(
      EDIT_AMAX(A, 100), EDIT_AMAX(B, 100), EDIT_AMAX(C, 10),
      EDIT_AMAX(I,  10), EDIT_AMAX(J,  10), EDIT_AMAX(K, 10),
      EDIT_AMAX(U,  10), EDIT_AMAX(V,  10), EDIT_AMAX(W, 10)
    );

    #if ENABLED(DISTINCT_E_FACTORS)
      EDIT_ITEM_FAST(long5_25, MSG_AMAX_E, &planner.settings.max_acceleration_mm_per_s2[E_AXIS_N(active_extruder)], 100, max_accel_edit_scaled.e, []{ planner.refresh_acceleration_rates(); });
      LOOP_L_N(n, E_STEPPERS)
        EDIT_ITEM_FAST_N(long5_25, n, MSG_AMAX_EN, &planner.settings.max_acceleration_mm_per_s2[E_AXIS_N(n)], 100, max_accel_edit_scaled.e, []{
          if (MenuItemBase::itemIndex == active_extruder)
            planner.refresh_acceleration_rates();
       });
    #elif E_STEPPERS
      EDIT_ITEM_FAST(long5_25, MSG_AMAX_E, &planner.settings.max_acceleration_mm_per_s2[E_AXIS], 100, max_accel_edit_scaled.e, []{ planner.refresh_acceleration_rates(); });
    #endif

    #ifdef XY_FREQUENCY_LIMIT
      EDIT_ITEM(int8, MSG_XY_FREQUENCY_LIMIT, &planner.xy_freq_limit_hz, 0, 100, planner.refresh_frequency_limit, true);
      editable.uint8 = uint8_t(LROUND(planner.xy_freq_min_speed_factor * 255)); // percent to u8
      EDIT_ITEM(percent, MSG_XY_FREQUENCY_FEEDRATE, &editable.uint8, 3, 255, []{ planner.set_min_speed_factor_u8(editable.uint8); }, true);
    #endif

    END_MENU();
  }

  #if HAS_CLASSIC_JERK

    void menu_advanced_jerk() {
      START_MENU();
      BACK_ITEM(MSG_ADVANCED_SETTINGS);

      #if HAS_JUNCTION_DEVIATION
        EDIT_ITEM(float43, MSG_JUNCTION_DEVIATION, &planner.junction_deviation_mm, 0.001f, TERN(LIN_ADVANCE, 0.3f, 0.5f)
          OPTARG(LIN_ADVANCE, planner.recalculate_max_e_jerk)
        );
      #endif

      constexpr xyze_float_t max_jerk_edit =
        #ifdef MAX_JERK_EDIT_VALUES
          MAX_JERK_EDIT_VALUES
        #elif ENABLED(LIMITED_JERK_EDITING)
          #define _JERK2(N) DEFAULT_##N##JERK * 2
          { MAPLIST(_JERK2, LOGICAL_AXIS_NAMES) }
        #else
          LOGICAL_AXIS_ARRAY_1(990)
        #endif
      ;

      LOOP_LOGICAL_AXES(a) {
        if (a == C_AXIS)
          EDIT_ITEM_FAST_N(float52sign, a, MSG_VN_JERK, &planner.max_jerk[a], 0.1f, max_jerk_edit[a]);
        else
          EDIT_ITEM_FAST_N(float3, a, MSG_VN_JERK, &planner.max_jerk[a], 1.0f, max_jerk_edit[a]);
      }

      END_MENU();
    }

  #endif

  // M851 - Z Probe Offsets
  #if HAS_BED_PROBE
    void menu_probe_offsets() {
      START_MENU();
      BACK_ITEM(MSG_ADVANCED_SETTINGS);
      #if HAS_PROBE_XY_OFFSET
        EDIT_ITEM(float31sign, MSG_ZPROBE_XOFFSET, &probe.offset.x, -(X_BED_SIZE), X_BED_SIZE);
        EDIT_ITEM(float31sign, MSG_ZPROBE_YOFFSET, &probe.offset.y, -(Y_BED_SIZE), Y_BED_SIZE);
      #endif
      EDIT_ITEM(LCD_Z_OFFSET_TYPE, MSG_ZPROBE_ZOFFSET, &probe.offset.z, Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX);

      #if ENABLED(PROBE_OFFSET_WIZARD)
        SUBMENU(MSG_PROBE_WIZARD, goto_probe_offset_wizard);
      #endif

      #if ENABLED(X_AXIS_TWIST_COMPENSATION)
        SUBMENU(MSG_XATC, xatc_wizard_continue);
      #endif

      END_MENU();
    }
  #endif

#endif // !SLIM_LCD_MENUS

// M92 Steps-per-mm
void menu_advanced_steps_per_mm() {
  START_MENU();
  BACK_ITEM(MSG_ADVANCED_SETTINGS);

  LOOP_NUM_AXES(a)
    EDIT_ITEM_FAST_N(float61, a, MSG_N_STEPS, &planner.settings.axis_steps_per_mm[a], 5, 9999, []{ planner.refresh_positioning(); });

  #if ENABLED(DISTINCT_E_FACTORS)
    LOOP_L_N(n, E_STEPPERS)
      EDIT_ITEM_FAST_N(float61, n, MSG_EN_STEPS, &planner.settings.axis_steps_per_mm[E_AXIS_N(n)], 5, 9999, []{
        const uint8_t e = MenuItemBase::itemIndex;
        if (e == active_extruder)
          planner.refresh_positioning();
        else
          planner.mm_per_step[E_AXIS_N(e)] = 1.0f / planner.settings.axis_steps_per_mm[E_AXIS_N(e)];
      });
  #elif E_STEPPERS
    EDIT_ITEM_FAST_N(float61, E_AXIS, MSG_N_STEPS, &planner.settings.axis_steps_per_mm[E_AXIS], 5, 9999, []{ planner.refresh_positioning(); });
  #endif

  END_MENU();
}

void menu_advanced_settings() {
  const bool is_busy = printer_busy();

  #if ENABLED(SD_FIRMWARE_UPDATE)
    bool sd_update_state = settings.sd_update_status();
  #endif

  START_MENU();
  BACK_ITEM(MSG_CONFIGURATION);

  #if DISABLED(SLIM_LCD_MENUS)

    // M203 / M205 - Feedrate items
    SUBMENU(MSG_MAX_SPEED, menu_advanced_velocity);

    // M201 - Acceleration items
    SUBMENU(MSG_ACCELERATION, menu_advanced_acceleration);

    #if HAS_CLASSIC_JERK
      // M205 - Max Jerk
      SUBMENU(MSG_JERK, menu_advanced_jerk);
    #elif HAS_JUNCTION_DEVIATION
      EDIT_ITEM(float43, MSG_JUNCTION_DEVIATION, &planner.junction_deviation_mm, 0.001f, 0.3f
        OPTARG(LIN_ADVANCE, planner.recalculate_max_e_jerk)
      );
    #endif

    // M851 - Z Probe Offsets
    #if HAS_BED_PROBE
      if (!is_busy) SUBMENU(MSG_ZPROBE_OFFSETS, menu_probe_offsets);
    #endif

  #endif // !SLIM_LCD_MENUS

  // M92 - Steps Per mm
  if (!is_busy)
    SUBMENU(MSG_STEPS_PER_MM, menu_advanced_steps_per_mm);

  #if ENABLED(BACKLASH_GCODE)
    SUBMENU(MSG_BACKLASH, menu_backlash);
  #endif

  #if HAS_MOTOR_CURRENT_DAC
    SUBMENU(MSG_DRIVE_STRENGTH, menu_dac);
  #endif
  #if HAS_MOTOR_CURRENT_PWM
    SUBMENU(MSG_DRIVE_STRENGTH, menu_pwm);
  #endif

  #if SHOW_MENU_ADVANCED_TEMPERATURE
    SUBMENU(MSG_TEMPERATURE, menu_advanced_temperature);
  #endif

  #if ENABLED(SD_FIRMWARE_UPDATE)
    EDIT_ITEM(bool, MSG_MEDIA_UPDATE, &sd_update_state, []{
      //
      // Toggle the SD Firmware Update state in EEPROM
      //
      const bool new_state = !settings.sd_update_status(),
                 didset = settings.set_sd_update_status(new_state);
      ui.completion_feedback(didset);
      ui.return_to_status();
      if (new_state) LCD_MESSAGE(MSG_RESET_PRINTER); else ui.reset_status();
    });
  #endif

  #if ENABLED(EEPROM_SETTINGS) && DISABLED(SLIM_LCD_MENUS)
    CONFIRM_ITEM(MSG_INIT_EEPROM,
      MSG_BUTTON_INIT, MSG_BUTTON_CANCEL,
      ui.init_eeprom, nullptr,
      GET_TEXT_F(MSG_INIT_EEPROM), (const char *)nullptr, F("?")
    );
  #endif

  END_MENU();
}

#endif // HAS_MARLINUI_MENU
