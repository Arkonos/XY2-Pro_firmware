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

/**
 * settings.cpp
 *
 * Settings and EEPROM storage
 *
 * IMPORTANT:  Whenever there are changes made to the variables stored in EEPROM
 * in the functions below, also increment the version number. This makes sure that
 * the default values are used whenever there is a change to the data, to prevent
 * wrong data being written to the variables.
 *
 * ALSO: Variables in the Store and Retrieve sections must be in the same order.
 *       If a feature is disabled, some data must still be written that, when read,
 *       either sets a Sane Default, or results in No Change to the existing value.
 */

// Change EEPROM version if the structure changes
#define EEPROM_OFFSET 100

// Check the integrity of data offsets.
// Can be disabled for production build.
//#define DEBUG_EEPROM_READWRITE

#include "settings.h"
#include "endstops.h"
#include "planner.h"
#include "stepper.h"
#include "temperature.h"

#include "../lcd/marlinui.h"
#include "../libs/vector_3.h"   // for matrix_3x3
#include "../gcode/gcode.h"
#include "../MarlinCore.h"

#if EITHER(EEPROM_SETTINGS, SD_FIRMWARE_UPDATE)
  #include "../HAL/shared/eeprom_api.h"
#endif

#include "probe.h"

#if HAS_LEVELING
  #include "../feature/bedlevel/bedlevel.h"
  #if ENABLED(X_AXIS_TWIST_COMPENSATION)
    #include "../feature/x_twist.h"
  #endif
#endif

#if ENABLED(Z_STEPPER_AUTO_ALIGN)
  #include "../feature/z_stepper_align.h"
#endif

#if ENABLED(EXTENSIBLE_UI)
  #include "../lcd/extui/ui_api.h"
#elif ENABLED(DWIN_LCD_PROUI)
  #include "../lcd/e3v2/proui/dwin.h"
  #include "../lcd/e3v2/proui/bedlevel_tools.h"
#elif ENABLED(DWIN_CREALITY_LCD_JYERSUI)
  #include "../lcd/e3v2/jyersui/dwin.h"
#endif

#if ENABLED(HOST_PROMPT_SUPPORT)
  #include "../feature/host_actions.h"
#endif


#if HAS_SERVOS && HAS_SERVO_ANGLES
  #define EEPROM_NUM_SERVOS NUM_SERVOS
#else
  #define EEPROM_NUM_SERVOS NUM_SERVO_PLUGS
#endif

#if HAS_POWER_MONITOR
  #include "../feature/power_monitor.h"
#endif

#if ENABLED(BACKLASH_COMPENSATION)
  #include "../feature/backlash.h"
#endif

#include "../feature/controllerfan.h"

#if ENABLED(TOUCH_SCREEN_CALIBRATION)
  #include "../lcd/tft_io/touch_calibration.h"
#endif

#if ENABLED(SOUND_MENU_ITEM)
  #include "../libs/buzzer.h"
#endif

#if HAS_FANCHECK
  #include "../feature/fancheck.h"
#endif

#if ENABLED(DGUS_LCD_UI_MKS)
  #include "../lcd/extui/dgus/DGUSScreenHandler.h"
  #include "../lcd/extui/dgus/DGUSDisplayDef.h"
#endif

#pragma pack(push, 1) // No padding between variables

#define _EN_ITEM(N) , E##N
#define _EN1_ITEM(N) , E##N:1

typedef struct { uint16_t MAIN_AXIS_NAMES, X2, Y2, Z2, Z3, Z4 REPEAT(E_STEPPERS, _EN_ITEM); } per_stepper_uint16_t;
typedef struct { uint32_t MAIN_AXIS_NAMES, X2, Y2, Z2, Z3, Z4 REPEAT(E_STEPPERS, _EN_ITEM); } per_stepper_uint32_t;
typedef struct {  int16_t MAIN_AXIS_NAMES, X2, Y2, Z2, Z3, Z4;                              } mot_stepper_int16_t;
typedef struct {     bool NUM_AXIS_LIST(X:1, Y:1, Z:1, I:1, J:1, K:1, U:1, V:1, W:1), X2:1, Y2:1, Z2:1, Z3:1, Z4:1 REPEAT(E_STEPPERS, _EN1_ITEM); } per_stepper_bool_t;

#undef _EN_ITEM

// Limit an index to an array size
#define ALIM(I,ARR) _MIN(I, (signed)COUNT(ARR) - 1)

// Defaults for reset / fill in on load
static const uint32_t   _DMA[] PROGMEM = DEFAULT_MAX_ACCELERATION;
static const float     _DASU[] PROGMEM = DEFAULT_AXIS_STEPS_PER_UNIT;
static const feedRate_t _DMF[] PROGMEM = DEFAULT_MAX_FEEDRATE;

/**
 * Current EEPROM Layout
 *
 * Keep this data structure up to date so
 * EEPROM size is known at compile time!
 */
typedef struct SettingsDataStruct {
  uint8_t   version[4];
  #ifndef EEPROM_AT24CXX 
    uint16_t  crc;                                        // Data Checksum
  #endif

  //
  // HAS_BED_PROBE
  //

  xyz_pos_t probe_offset;

  //
  // Hotend Offset
  //

  xyz_pos_t hotend_offset;

  //
  // AUTO_BED_LEVELING_BILINEAR
  //
  uint8_t grid_max_x, grid_max_y;                       // GRID_MAX_POINTS_X, GRID_MAX_POINTS_Y
  xy_pos_t bilinear_grid_spacing, bilinear_start;       // G29 L F
  #if ENABLED(AUTO_BED_LEVELING_BILINEAR)
    bed_mesh_t z_values;                                // G29
  #else
    float z_values[3][3];
  #endif

  //
  // DISTINCT_E_FACTORS
  //
  uint8_t e_factors;                                    // DISTINCT_AXES - NUM_AXES

  //
  // Planner settings
  //
  planner_settings_t planner_settings;

  xyze_float_t planner_max_jerk;                        // M205 XYZE  planner.max_jerk
  float planner_junction_deviation_mm;                  // M205 J     planner.junction_deviation_mm

  //
  // Home Offset
  //
  xyz_pos_t home_offset;                                // M206 XYZ / M665 TPZ

  //
  // FILAMENT_RUNOUT_SENSOR
  //
  bool runout_sensor_enabled;                           // M412 S
  float runout_distance_mm;                             // M412 D

  //
  // ENABLE_LEVELING_FADE_HEIGHT
  //
  float planner_z_fade_height;                          // M420 Zn  planner.z_fade_height

  //
  // MESH_BED_LEVELING
  //
  float mbl_z_offset;                                   // bedlevel.z_offset
  uint8_t mesh_num_x, mesh_num_y;                       // GRID_MAX_POINTS_X, GRID_MAX_POINTS_Y
  float mbl_z_values[TERN(MESH_BED_LEVELING, GRID_MAX_POINTS_X, 3)]   // bedlevel.z_values
                    [TERN(MESH_BED_LEVELING, GRID_MAX_POINTS_Y, 3)];

  //
  // ABL_PLANAR
  //
  matrix_3x3 planner_bed_level_matrix;                  // planner.bed_level_matrix

  //
  // X_AXIS_TWIST_COMPENSATION
  //
  #if ENABLED(X_AXIS_TWIST_COMPENSATION)
    float xatc_spacing;                                 // M423 X Z
    float xatc_start;
    xatc_array_t xatc_z_offset;
  #endif

  //
  // AUTO_BED_LEVELING_UBL
  //
  bool planner_leveling_active;                         // M420 S  planner.leveling_active
  int8_t ubl_storage_slot;                              // bedlevel.storage_slot

  //
  // Extra Endstops offsets
  //
  #if HAS_EXTRA_ENDSTOPS
    float x2_endstop_adj,                               // M666 X
          y2_endstop_adj,                               // M666 Y
          z2_endstop_adj,                               // M666 (S2) Z
          z3_endstop_adj,                               // M666 (S3) Z
          z4_endstop_adj;                               // M666 (S4) Z
  #endif

  //
  // Z_STEPPER_AUTO_ALIGN, HAS_Z_STEPPER_ALIGN_STEPPER_XY
  //
  #if ENABLED(Z_STEPPER_AUTO_ALIGN)
    xy_pos_t z_stepper_align_xy[NUM_Z_STEPPERS];             // M422 S X Y
    #if HAS_Z_STEPPER_ALIGN_STEPPER_XY
      xy_pos_t z_stepper_align_stepper_xy[NUM_Z_STEPPERS];   // M422 W X Y
    #endif
  #endif

  //
  // Material Presets
  //
  #if HAS_PREHEAT
    preheat_t ui_material_preset[PREHEAT_COUNT];        // M145 S0 H B F
  #endif

  //
  // User-defined Thermistors
  //
  #if HAS_USER_THERMISTORS
    user_thermistor_t user_thermistor[USER_THERMISTORS]; // M305 P0 R4700 T100000 B3950
  #endif

  //
  // Power monitor
  //
  uint8_t power_monitor_flags;                          // M430 I V W

  //
  // HAS_LCD_CONTRAST
  //
  uint8_t lcd_contrast;                                 // M250 C

  //
  // HAS_LCD_BRIGHTNESS
  //
  uint8_t lcd_brightness;                               // M256 B

  //
  // Display Sleep
  //
  #if LCD_BACKLIGHT_TIMEOUT
    uint16_t lcd_backlight_timeout;                     // M255 S
  #elif HAS_DISPLAY_SLEEP
    uint8_t sleep_timeout_minutes;                      // M255 S
  #endif

  //
  // Controller fan settings
  //
  controllerFan_settings_t controllerFan_settings;      // M710

  //
  // !NO_VOLUMETRIC
  //
  bool parser_volumetric_enabled;                       // M200 S  parser.volumetric_enabled

  //
  // HAS_MOTOR_CURRENT_PWM
  //
  #ifndef MOTOR_CURRENT_COUNT
    #if HAS_MOTOR_CURRENT_PWM
      #define MOTOR_CURRENT_COUNT 3
    #elif HAS_MOTOR_CURRENT_DAC
      #define MOTOR_CURRENT_COUNT LOGICAL_AXES
    #elif HAS_MOTOR_CURRENT_I2C
      #define MOTOR_CURRENT_COUNT DIGIPOT_I2C_NUM_CHANNELS
    #else // HAS_MOTOR_CURRENT_SPI
      #define MOTOR_CURRENT_COUNT DISTINCT_AXES
    #endif
  #endif
  uint32_t motor_current_setting[MOTOR_CURRENT_COUNT];  // M907 X Z E ...

  //
  // CNC_COORDINATE_SYSTEMS
  //
  xyz_pos_t coordinate_system[MAX_COORDINATE_SYSTEMS];  // G54-G59.3

  //
  // SKEW_CORRECTION
  //
  skew_factor_t planner_skew_factor;                    // M852 I J K  planner.skew_factor

   //
  // BACKLASH_COMPENSATION
  //
  xyz_float_t backlash_distance_mm;                     // M425 X Y Z
  uint8_t backlash_correction;                          // M425 F
  float backlash_smoothing_mm;                          // M425 S

  //
  // EXTENSIBLE_UI
  //
  #if ENABLED(EXTENSIBLE_UI)
    uint8_t extui_data[ExtUI::eeprom_data_size];
  #endif

  //
  // CASELIGHT_USES_BRIGHTNESS
  //
  #if CASELIGHT_USES_BRIGHTNESS
    uint8_t caselight_brightness;                        // M355 P
  #endif

  //
  // TOUCH_SCREEN_CALIBRATION
  //
  #if ENABLED(TOUCH_SCREEN_CALIBRATION)
    touch_calibration_t touch_calibration_data;
  #endif

  //
  // Buzzer enable/disable
  //
  #if ENABLED(SOUND_MENU_ITEM)
    bool sound_on;
  #endif

  //
  // Fan tachometer check
  //
  #if HAS_FANCHECK
    bool fan_check_enabled;
  #endif

  //
  // MKS UI controller
  //
  #if ENABLED(DGUS_LCD_UI_MKS)
    MKS_Language mks_language_index;                    // Display Language
    xy_int_t mks_corner_offsets[5];                     // Bed Tramming
    xyz_int_t mks_park_pos;                             // Custom Parking (without NOZZLE_PARK)
    celsius_t mks_min_extrusion_temp;                   // Min E Temp (shadow M302 value)
  #endif

  #if HAS_MULTI_LANGUAGE
    uint8_t ui_language;                                // M414 S
  #endif

  //
  // Model predictive control
  //
  #if ENABLED(MPCTEMP)
    MPC_t mpc_constants[HOTENDS];                       // M306
  #endif

} SettingsData;

//static_assert(sizeof(SettingsData) <= MARLIN_EEPROM_SIZE, "EEPROM too small to contain SettingsData!");

MarlinSettings settings;

uint16_t MarlinSettings::datasize() { return sizeof(SettingsData); }

/**
 * Post-process after Retrieve or Reset
 */

#if ENABLED(ENABLE_LEVELING_FADE_HEIGHT)
  float new_z_fade_height;
#endif

void MarlinSettings::postprocess() {
  xyze_pos_t oldpos = current_position;

  // steps per s2 needs to be updated to agree with units per s2
  planner.refresh_acceleration_rates();

  // Software endstops depend on home_offset
  LOOP_NUM_AXES(i) {
    update_workspace_offset((AxisEnum)i);
    update_software_endstops((AxisEnum)i);
  }

  TERN_(ENABLE_LEVELING_FADE_HEIGHT, set_z_fade_height(new_z_fade_height, false)); // false = no report

  TERN_(AUTO_BED_LEVELING_BILINEAR, bedlevel.refresh_bed_level());

  TERN_(HAS_MOTOR_CURRENT_PWM, stepper.refresh_motor_power());

  TERN_(EXTENSIBLE_UI, ExtUI::onPostprocessSettings());

  // Refresh mm_per_step with the reciprocal of axis_steps_per_mm
  // and init stepper.count[], planner.position[] with current_position
  planner.refresh_positioning();

  // Various factors can change the current position
  if (oldpos != current_position)
    report_current_position();

  // Moved as last update due to interference with Neopixel init
  TERN_(HAS_LCD_CONTRAST, ui.refresh_contrast());
  TERN_(HAS_LCD_BRIGHTNESS, ui.refresh_brightness());

  #if LCD_BACKLIGHT_TIMEOUT
    ui.refresh_backlight_timeout();
  #elif HAS_DISPLAY_SLEEP
    ui.refresh_screen_timeout();
  #endif
}

#ifdef ARCHIM2_SPI_FLASH_EEPROM_BACKUP_SIZE
  static_assert(EEPROM_OFFSET + sizeof(SettingsData) < ARCHIM2_SPI_FLASH_EEPROM_BACKUP_SIZE,
                "ARCHIM2_SPI_FLASH_EEPROM_BACKUP_SIZE is insufficient to capture all EEPROM data.");
#endif

//
// This file simply uses the DEBUG_ECHO macros to implement EEPROM_CHITCHAT.
// For deeper debugging of EEPROM issues enable DEBUG_EEPROM_READWRITE.
//
#define DEBUG_OUT EITHER(EEPROM_CHITCHAT, DEBUG_LEVELING_FEATURE)
#include "../core/debug_out.h"

#if BOTH(EEPROM_CHITCHAT, HOST_PROMPT_SUPPORT)
  #define HOST_EEPROM_CHITCHAT 1
#endif

/**
 * M502 - Reset Configuration
 */
void MarlinSettings::reset() {
  LOOP_DISTINCT_AXES(i) {
    planner.settings.max_acceleration_mm_per_s2[i] = pgm_read_dword(&_DMA[ALIM(i, _DMA)]);
    planner.settings.axis_steps_per_mm[i] = pgm_read_float(&_DASU[ALIM(i, _DASU)]);
    planner.settings.max_feedrate_mm_s[i] = pgm_read_float(&_DMF[ALIM(i, _DMF)]);
  }

  planner.settings.min_segment_time_us = DEFAULT_MINSEGMENTTIME;
  planner.settings.acceleration = DEFAULT_ACCELERATION;
  planner.settings.travel_acceleration = DEFAULT_TRAVEL_ACCELERATION;
  planner.settings.min_feedrate_mm_s = feedRate_t(DEFAULT_MINIMUMFEEDRATE);
  planner.settings.min_travel_feedrate_mm_s = feedRate_t(DEFAULT_MINTRAVELFEEDRATE);

  #if HAS_CLASSIC_JERK
    planner.max_jerk.set(
      NUM_AXIS_LIST(DEFAULT_XJERK, DEFAULT_YJERK, DEFAULT_ZJERK)
    );
  #endif

  #if HAS_HOME_OFFSET
    home_offset.reset();
  #endif

  #if ENABLED(BACKLASH_GCODE)
    backlash.set_correction(BACKLASH_CORRECTION);
    constexpr xyz_float_t tmp = BACKLASH_DISTANCE_MM;
    LOOP_NUM_AXES(axis) backlash.set_distance_mm((AxisEnum)axis, tmp[axis]);
    #ifdef BACKLASH_SMOOTHING_MM
      backlash.set_smoothing_mm(BACKLASH_SMOOTHING_MM);
    #endif
  #endif

  //
  // TOUCH_SCREEN_CALIBRATION
  //
  TERN_(TOUCH_SCREEN_CALIBRATION, touch_calibration.calibration_reset());

  //
  // Buzzer enable/disable
  //
  #if ENABLED(SOUND_MENU_ITEM)
    ui.sound_on = ENABLED(SOUND_ON_DEFAULT);
  #endif


  //
  // Global Leveling
  //
  TERN_(ENABLE_LEVELING_FADE_HEIGHT, new_z_fade_height = (DEFAULT_LEVELING_FADE_HEIGHT));
  TERN_(HAS_LEVELING, reset_bed_level());

  //
  // X Axis Twist Compensation
  //
  TERN_(X_AXIS_TWIST_COMPENSATION, xatc.reset());

  //
  // Nozzle-to-probe Offset
  //
  #if HAS_BED_PROBE
    constexpr float dpo[] = NOZZLE_TO_PROBE_OFFSET;
    static_assert(COUNT(dpo) == NUM_AXES, "NOZZLE_TO_PROBE_OFFSET must contain offsets for each linear axis X, Y, Z....");
    #if HAS_PROBE_XY_OFFSET
      LOOP_NUM_AXES(a) probe.offset[a] = dpo[a];
    #else
      probe.offset.set(NUM_AXIS_LIST(0, 0, dpo[Z_AXIS], 0, 0, 0, 0, 0, 0));
    #endif
  #endif

  //
  // Z Stepper Auto-alignment points
  //
  TERN_(Z_STEPPER_AUTO_ALIGN, z_stepper_align.reset_to_default());

  
  // Probe Temperature Compensation
  //
  TERN_(HAS_PTC, ptc.reset());

  //
  // Endstop Adjustments
  //

  #if ENABLED(Z_MULTI_ENDSTOPS)
    #ifndef Z2_ENDSTOP_ADJUSTMENT
      #define Z2_ENDSTOP_ADJUSTMENT 0
    #endif
    endstops.z2_endstop_adj = Z2_ENDSTOP_ADJUSTMENT;
    #if NUM_Z_STEPPERS >= 3
      #ifndef Z3_ENDSTOP_ADJUSTMENT
        #define Z3_ENDSTOP_ADJUSTMENT 0
      #endif
      endstops.z3_endstop_adj = Z3_ENDSTOP_ADJUSTMENT;
    #endif
    #if NUM_Z_STEPPERS >= 4
      #ifndef Z4_ENDSTOP_ADJUSTMENT
        #define Z4_ENDSTOP_ADJUSTMENT 0
      #endif
      endstops.z4_endstop_adj = Z4_ENDSTOP_ADJUSTMENT;
    #endif
  #endif

  //
  // Preheat parameters
  //
  #if HAS_PREHEAT
    #define _PITEM(N,T) PREHEAT_##N##_##T,
    #if HAS_HEATED_BED
      constexpr uint16_t bpre[] = { REPEAT2_S(1, INCREMENT(PREHEAT_COUNT), _PITEM, TEMP_BED) };
    #endif
    LOOP_L_N(i, PREHEAT_COUNT) {
      TERN_(HAS_HOTEND,     ui.material_preset[i].hotend_temp = hpre[i]);
      TERN_(HAS_HEATED_BED, ui.material_preset[i].bed_temp = bpre[i]);
      TERN_(HAS_FAN,        ui.material_preset[i].fan_speed = fpre[i]);
    }
  #endif

  //
  // Controller Fan
  //
  TERN_(USE_CONTROLLER_FAN, controllerFan.reset());

  endstops.enable_globally(ENABLED(ENDSTOPS_ALWAYS_ON_DEFAULT));

  reset_stepper_drivers();

  postprocess();

  #if EITHER(EEPROM_CHITCHAT, DEBUG_LEVELING_FEATURE)
    FSTR_P const hdsl = F("Hardcoded Default Settings Loaded");
    TERN_(HOST_EEPROM_CHITCHAT, hostui.notify(hdsl));
    DEBUG_ECHO_START(); DEBUG_ECHOLNF(hdsl);
  #endif
}

#if DISABLED(DISABLE_M503)

  #define CONFIG_ECHO_START()       gcode.report_echo_start(forReplay)
  #define CONFIG_ECHO_MSG(V...)     do{ CONFIG_ECHO_START(); SERIAL_ECHOLNPGM(V); }while(0)
  #define CONFIG_ECHO_MSG_P(V...)   do{ CONFIG_ECHO_START(); SERIAL_ECHOLNPGM_P(V); }while(0)
  #define CONFIG_ECHO_HEADING(STR)  gcode.report_heading(forReplay, F(STR))

  void M92_report(const bool echo=true, const int8_t e=-1);

  /**
   * M503 - Report current settings in RAM
   *
   * Unless specifically disabled, M503 is available even without EEPROM
   */
  void MarlinSettings::report(const bool forReplay) {
    #if REPORT_PARAMETER
    CONFIG_ECHO_HEADING("Linear Units");
    CONFIG_ECHO_START();
    SERIAL_ECHOPGM("  G21 ;");
    gcode.say_units(); // " (mm)"

    //
    // M149 Temperature units
    //
    #if ENABLED(TEMPERATURE_UNITS_SUPPORT)
      gcode.M149_report(forReplay);
    #else
      CONFIG_ECHO_HEADING(STR_TEMPERATURE_UNITS);
      CONFIG_ECHO_MSG("  M149 C ; Units in Celsius");
    #endif

    //
    // M200 Volumetric Extrusion
    //
    IF_DISABLED(NO_VOLUMETRICS, gcode.M200_report(forReplay));

    //
    // M92 Steps per Unit
    //
    gcode.M92_report(forReplay);

    //
    // M203 Maximum feedrates (units/s)
    //
    gcode.M203_report(forReplay);

    //
    // M201 Maximum Acceleration (units/s2)
    //
    gcode.M201_report(forReplay);

    //
    // M204 Acceleration (units/s2)
    //
    gcode.M204_report(forReplay);

    //
    // M205 "Advanced" Settings
    //
    gcode.M205_report(forReplay);

    //
    // LCD Contrast
    //
    TERN_(HAS_LCD_CONTRAST, gcode.M250_report(forReplay));

    //
    // Display Sleep
    //
    TERN_(HAS_GCODE_M255, gcode.M255_report(forReplay));

    //
    // LCD Brightness
    //
    TERN_(HAS_LCD_BRIGHTNESS, gcode.M256_report(forReplay));

    //
    // Controller Fan
    //
    TERN_(CONTROLLER_FAN_EDITABLE, gcode.M710_report(forReplay));
    //
    // Probe Offset
    //
    TERN_(HAS_BED_PROBE, gcode.M851_report(forReplay));

    //
    // Bed Skew Correction
    //
    TERN_(SKEW_CORRECTION_GCODE, gcode.M852_report(forReplay));

    //
    // Motor Current (SPI or PWM)
    //
    #if HAS_MOTOR_CURRENT_SPI || HAS_MOTOR_CURRENT_PWM
      gcode.M907_report(forReplay);
    #endif

    //
    // Backlash Compensation
    //
    TERN_(BACKLASH_GCODE, gcode.M425_report(forReplay));
    TERN_(HAS_MULTI_LANGUAGE, gcode.M414_report(forReplay));
    #endif
  }

#endif // !DISABLE_M503

#pragma pack(pop)
