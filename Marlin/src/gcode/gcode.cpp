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
 * gcode.cpp - Temporary container for all G-code handlers
 *             Most will migrate to classes, by feature.
 */

#include "gcode.h"
GcodeSuite gcode;

#include "parser.h"
#include "queue.h"
#include "../module/motion.h"

#if ENABLED(HOST_ACTION_COMMANDS)
  #include "../feature/host_actions.h"
#endif

#if ENABLED(FLOWMETER_SAFETY)
  #include "../feature/cooler.h"
#endif

#if HAS_FANCHECK
  #include "../feature/fancheck.h"
#endif

#include "../MarlinCore.h" // for idle, kill

#include "../module/probe.h"
#include "../module/planner.h"
#include "../module/temperature.h"
#include "../feature/babystep.h"
#include "../module/settings.h"
#define IS_SPACE(x) (x == ' ' || x == '\t' || x == '\f' || x == '\r' || x == '\n')

// Inactivity shutdown
millis_t GcodeSuite::previous_move_ms = 0,
         GcodeSuite::max_inactive_time = 0;

// Relative motion mode for each logical axis
static constexpr xyze_bool_t ar_init = AXIS_RELATIVE_MODES;
axis_bits_t GcodeSuite::axis_relative = 0 LOGICAL_AXIS_GANG(
  | (ar_init.e << REL_E),
  | (ar_init.x << REL_X),
  | (ar_init.y << REL_Y),
  | (ar_init.z << REL_Z),
  | (ar_init.i << REL_I),
  | (ar_init.j << REL_J),
  | (ar_init.k << REL_K),
  | (ar_init.u << REL_U),
  | (ar_init.v << REL_V),
  | (ar_init.w << REL_W)
);

#if EITHER(HAS_AUTO_REPORTING, HOST_KEEPALIVE_FEATURE)
  bool GcodeSuite::autoreport_paused; // = false
#endif

#if ENABLED(HOST_KEEPALIVE_FEATURE)
  GcodeSuite::MarlinBusyState GcodeSuite::busy_state = NOT_BUSY;
  uint8_t GcodeSuite::host_keepalive_interval = DEFAULT_KEEPALIVE_INTERVAL;
#endif

#if ENABLED(CNC_WORKSPACE_PLANES)
  GcodeSuite::WorkspacePlane GcodeSuite::workspace_plane = PLANE_XY;
#endif

#if ENABLED(CNC_COORDINATE_SYSTEMS)
  int8_t GcodeSuite::active_coordinate_system = -1; // machine space
  xyz_pos_t GcodeSuite::coordinate_system[MAX_COORDINATE_SYSTEMS];
#endif

void GcodeSuite::report_echo_start(const bool forReplay) { if (!forReplay) SERIAL_ECHO_START(); }
void GcodeSuite::report_heading(const bool forReplay, FSTR_P const fstr, const bool eol/*=true*/) {
  if (forReplay) return;
  if (fstr) {
    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("; ");
    SERIAL_ECHOF(fstr);
  }
  if (eol) { SERIAL_CHAR(':'); SERIAL_EOL(); }
}

void GcodeSuite::say_units() {
  SERIAL_ECHOLNPGM_P(
    TERN_(INCH_MODE_SUPPORT, parser.linear_unit_factor != 1.0 ? PSTR(" (in)") :)
    PSTR(" (mm)")
  );
}

/**
 * Get the target extruder from the T parameter or the active_extruder
 * Return -1 if the T parameter is out of range
 */
int8_t GcodeSuite::get_target_extruder_from_command() {
  if (parser.seenval('T')) {
    const int8_t e = parser.value_byte();
    if (e < 0) return e;
    SERIAL_ECHO_START();
    SERIAL_CHAR('M'); SERIAL_ECHO(parser.codenum);
    SERIAL_ECHOLNPGM(" " STR_INVALID_EXTRUDER " ", e);
    return -1;
  }
  return active_extruder;
}

/**
 * Get the target E stepper from the 'T' parameter.
 * If there is no 'T' parameter then dval will be substituted.
 * Returns -1 if the resulting E stepper index is out of range.
 */
int8_t GcodeSuite::get_target_e_stepper_from_command(const int8_t dval/*=-1*/) {
  const int8_t e = parser.intval('T', dval);
  if (WITHIN(e, 0, E_STEPPERS - 1)) return e;
  if (dval == -2) return dval;

  SERIAL_ECHO_START();
  SERIAL_CHAR('M'); SERIAL_ECHO(parser.codenum);
  if (e == -1)
    SERIAL_ECHOLNPGM(" " STR_E_STEPPER_NOT_SPECIFIED);
  else
    SERIAL_ECHOLNPGM(" " STR_INVALID_E_STEPPER " ", e);
  return -1;
}

/**
 * Set XYZ...E destination and feedrate from the current GCode command
 *
 *  - Set destination from included axis codes
 *  - Set to current for missing axis codes
 *  - Set the feedrate, if included
 */
void GcodeSuite::get_destination_from_command() {
  xyze_bool_t seen{false};

  #if ENABLED(CANCEL_OBJECTS)
    const bool &skip_move = cancelable.skipping;
  #else
    constexpr bool skip_move = false;
  #endif

  // Get new XYZ position, whether absolute or relative
  LOOP_NUM_AXES(i) {
    if ( (seen[i] = parser.seenval(AXIS_CHAR(i))) ) {
      const float v = parser.value_axis_units((AxisEnum)i);
      if (skip_move)
        destination[i] = current_position[i];
      else
        destination[i] = axis_is_relative(AxisEnum(i)) ? current_position[i] + v : LOGICAL_TO_NATIVE(v, i);
    }
    else
      destination[i] = current_position[i];
  }

  if (parser.floatval('F') > 0) {
    feedrate_mm_s = parser.value_feedrate();
  }
}

/**
 * Dwell waits immediately. It does not synchronize. Use M400 instead of G4
 */
void GcodeSuite::dwell(millis_t time) {
  time += millis();
  while (PENDING(millis(), time)) idle();
}

/**
 * When G29_RETRY_AND_RECOVER is enabled, call G29() in
 * a loop with recovery and retry handling.
 */
#if ENABLED(G29_RETRY_AND_RECOVER)

  void GcodeSuite::event_probe_recover() {
    TERN_(HOST_PROMPT_SUPPORT, hostui.prompt_do(PROMPT_INFO, F("G29 Retrying"), FPSTR(DISMISS_STR)));
    #ifdef ACTION_ON_G29_RECOVER
      hostui.g29_recover();
    #endif
    #ifdef G29_RECOVER_COMMANDS
      process_subcommands_now(F(G29_RECOVER_COMMANDS));
    #endif
  }

  #if ENABLED(G29_HALT_ON_FAILURE)
    #include "../lcd/marlinui.h"
  #endif

  void GcodeSuite::event_probe_failure() {
    #ifdef ACTION_ON_G29_FAILURE
      hostui.g29_failure();
    #endif
    #ifdef G29_FAILURE_COMMANDS
      process_subcommands_now(F(G29_FAILURE_COMMANDS));
    #endif
    #if ENABLED(G29_HALT_ON_FAILURE)
      #ifdef ACTION_ON_CANCEL
        hostui.cancel();
      #endif
      kill(GET_TEXT_F(MSG_LCD_PROBING_FAILED));
    #endif
  }

  #ifndef G29_MAX_RETRIES
    #define G29_MAX_RETRIES 0
  #endif

  void GcodeSuite::G29_with_retry() {
    uint8_t retries = G29_MAX_RETRIES;
    while (G29()) { // G29 should return true for failed probes ONLY
      if (retries) {
        event_probe_recover();
        --retries;
      }
      else {
        event_probe_failure();
        return;
      }
    }

    TERN_(HOST_PROMPT_SUPPORT, hostui.prompt_end());

    #ifdef G29_SUCCESS_COMMANDS
      process_subcommands_now(F(G29_SUCCESS_COMMANDS));
    #endif
  }

#endif // G29_RETRY_AND_RECOVER

/**
 * Process the parsed command and dispatch it to its handler
 */
void GcodeSuite::process_parsed_command(const bool no_ok/*=false*/) {
  TERN_(HAS_FANCHECK, fan_check.check_deferred_error());

  KEEPALIVE_STATE(IN_HANDLER);

 /**
  * Block all Gcodes except M511 Unlock Printer, if printer is locked
  * Will still block Gcodes if M511 is disabled, in which case the printer should be unlocked via LCD Menu
  */
  #if ENABLED(PASSWORD_FEATURE)
    if (password.is_locked && !parser.is_command('M', 511)) {
      SERIAL_ECHO_MSG(STR_PRINTER_LOCKED);
      if (!no_ok) queue.ok_to_send();
      return;
    }
  #endif

  #if ENABLED(FLOWMETER_SAFETY)
    if (cooler.flowfault) {
      SERIAL_ECHO_MSG(STR_FLOWMETER_FAULT);
      return;
    }
  #endif

  // Handle a known command or reply "unknown command"

  switch (parser.command_letter) {

    case 'G': switch (parser.codenum) {

      case 0: case 1:                                             // G0: Fast Move, G1: Linear Move
        G0_G1(TERN_(HAS_FAST_MOVES, parser.codenum == 0)); break;

      #if ENABLED(ARC_SUPPORT)
        case 2: case 3: G2_G3(parser.codenum == 2); break;        // G2: CW ARC, G3: CCW ARC
      #endif

      case 4: G4(); break;                                        // G4: Dwell

      #if ENABLED(BEZIER_CURVE_SUPPORT)
        case 5: G5(); break;                                      // G5: Cubic B_spline
      #endif

      #if ENABLED(DIRECT_STEPPING)
        case 6: G6(); break;                                      // G6: Direct Stepper Move
      #endif


      #if ENABLED(NOZZLE_CLEAN_FEATURE)
        case 12: G12(); break;                                    // G12: Nozzle Clean
      #endif

      #if ENABLED(CNC_WORKSPACE_PLANES)
        case 17: G17(); break;                                    // G17: Select Plane XY
        case 18: G18(); break;                                    // G18: Select Plane ZX
        case 19: G19(); break;                                    // G19: Select Plane YZ
      #endif

      #if ENABLED(INCH_MODE_SUPPORT)
        case 20: G20(); break;                                    // G20: Inch Mode
        case 21: G21(); break;                                    // G21: MM Mode
      #else
        case 21: NOOP; break;                                     // No error on unknown G21
      #endif

      #if ENABLED(G26_MESH_VALIDATION)
        case 26: G26(); break;                                    // G26: Mesh Validation Pattern generation
      #endif

      case 28: G28(); break;                                      // G28: Home one or more axes

      #if HAS_LEVELING
        case 29:                                                  // G29: Bed leveling calibration
          TERN(G29_RETRY_AND_RECOVER, G29_with_retry, G29)();
          break;
      #endif

      #if HAS_BED_PROBE
        case 30: G30(); break;                                    // G30: Single Z probe
        #if ENABLED(Z_PROBE_SLED)
          case 31: G31(); break;                                  // G31: dock the sled
          case 32: G32(); break;                                  // G32: undock the sled
        #endif
      #endif

      #if ENABLED(DELTA_AUTO_CALIBRATION)
        case 33: G33(); break;                                    // G33: Delta Auto-Calibration
      #endif

      #if ANY(Z_MULTI_ENDSTOPS, Z_STEPPER_AUTO_ALIGN, MECHANICAL_GANTRY_CALIBRATION)
        case 34: G34(); break;                                    // G34: Z Stepper automatic alignment using probe
      #endif

      #if ENABLED(ASSISTED_TRAMMING)
        case 35: G35(); break;                                    // G35: Read four bed corners to help adjust bed screws
      #endif

      #if ENABLED(G38_PROBE_TARGET)
        case 38:                                                  // G38.2, G38.3: Probe towards target
          if (WITHIN(parser.subcode, 2, TERN(G38_PROBE_AWAY, 5, 3)))
            G38(parser.subcode);                                  // G38.4, G38.5: Probe away from target
          break;
      #endif

      #if HAS_MESH
        case 42: G42(); break;                                    // G42: Coordinated move to a mesh point
      #endif

      #if ENABLED(CNC_COORDINATE_SYSTEMS)
        case 53: G53(); break;                                    // G53: (prefix) Apply native workspace
        case 54: G54(); break;                                    // G54: Switch to Workspace 1
        case 55: G55(); break;                                    // G55: Switch to Workspace 2
        case 56: G56(); break;                                    // G56: Switch to Workspace 3
        case 57: G57(); break;                                    // G57: Switch to Workspace 4
        case 58: G58(); break;                                    // G58: Switch to Workspace 5
        case 59: G59(); break;                                    // G59.0 - G59.3: Switch to Workspace 6-9
      #endif

      #if SAVED_POSITIONS
        case 60: G60(); break;                                    // G60:  save current position
        case 61: G61(); break;                                    // G61:  Apply/restore saved coordinates.
      #endif

      #if BOTH(PTC_PROBE, PTC_BED)
        case 76: G76(); break;                                    // G76: Calibrate first layer compensation values
      #endif

      #if ENABLED(GCODE_MOTION_MODES)
        case 80: G80(); break;                                    // G80: Reset the current motion mode
      #endif

      case 90: set_relative_mode(false); break;                   // G90: Absolute Mode
      case 91: set_relative_mode(true);  break;                   // G91: Relative Mode

      case 92: G92(); break;                                      // G92: Set current axis position(s)

      #if ENABLED(CALIBRATION_GCODE)
        case 425: G425(); break;                                  // G425: Perform calibration with calibration cube
      #endif

      #if ENABLED(DEBUG_GCODE_PARSER)
        case 800: parser.debug(); break;                          // G800: GCode Parser Test for G
      #endif

      default: parser.unknown_command_warning(); break;
    }
    break;

    case 'M': switch (parser.codenum) {

      #if HAS_RESUME_CONTINUE
        case 0:                                                   // M0: Unconditional stop - Wait for user button press on LCD
        case 1: M0_M1(); break;                                   // M1: Conditional stop - Wait for user button press on LCD
      #endif

      #if ENABLED(EXTERNAL_CLOSED_LOOP_CONTROLLER)
        case 12: M12(); break;                                    // M12: Synchronize and optionally force a CLC set
      #endif

      #if ENABLED(EXPECTED_PRINTER_CHECK)
        case 16: M16(); break;                                    // M16: Expected printer check
      #endif

      case 17: M17(); break;                                      // M17: Enable all stepper motors

      #if ENABLED(DIRECT_PIN_CONTROL)
        case 42: M42(); break;                                    // M42: Change pin state
      #endif

      #if ENABLED(PINS_DEBUGGING)
        case 43: M43(); break;                                    // M43: Read pin state
      #endif

      #if ENABLED(Z_MIN_PROBE_REPEATABILITY_TEST)
        case 48: M48(); break;                                    // M48: Z probe repeatability test
      #endif

      #if ENABLED(M100_FREE_MEMORY_WATCHER)
        case 100: M100(); break;                                  // M100: Free Memory Report
      #endif

      case 105: M105(); return;                                   // M105: Report Temperatures (and say "ok")

      case 110: M110(); break;                                    // M110: Set Current Line Number
      case 111: M111(); break;                                    // M111: Set debug level

      #if DISABLED(EMERGENCY_PARSER)
        case 108: M108(); break;                                  // M108: Cancel Waiting
        case 112: M112(); break;                                  // M112: Full Shutdown
        case 410: M410(); break;                                  // M410: Quickstop - Abort all the planned moves.
        TERN_(HOST_PROMPT_SUPPORT, case 876:)                     // M876: Handle Host prompt responses
      #else
        case 108: case 112: case 410:
        TERN_(HOST_PROMPT_SUPPORT, case 876:)
        break;
      #endif

      #if ENABLED(HOST_KEEPALIVE_FEATURE)
        case 113: M113(); break;                                  // M113: Set Host Keepalive interval
      #endif

      #if HAS_FANCHECK
        case 123: M123(); break;                                  // M123: Report fan states or set fans auto-report interval
      #endif

      #if HAS_HEATED_BED
        case 140: M140(); break;                                  // M140: Set bed temperature
        case 190: M190(); break;                                  // M190: Wait for bed temperature to reach target
      #endif

      #if HAS_TEMP_PROBE
        case 192: M192(); break;                                  // M192: Wait for probe temp
      #endif

      #if ENABLED(AUTO_REPORT_POSITION)
        case 154: M154(); break;                                  // M154: Set position auto-report interval
      #endif

      #if BOTH(AUTO_REPORT_TEMPERATURES, HAS_TEMP_SENSOR)
        case 155: M155(); break;                                  // M155: Set temperature auto-report interval
      #endif

      #if ENABLED(PSU_CONTROL)
        case 80: M80(); break;                                    // M80: Turn on Power Supply
      #endif
      case 81: M81(); break;                                      // M81: Turn off Power, including Power Supply, if possible

      case 18: case 84: M18_M84(); break;                         // M18/M84: Disable Steppers / Set Timeout
      case 92: M92(); break;                                      // M92: Set the steps-per-unit for one or more axes
      case 114: M114(); break;                                    // M114: Report current position
      case 115: M115(); break;                                    // M115: Report capabilities

      case 117: TERN_(HAS_STATUS_MESSAGE, M117()); break;         // M117: Set LCD message text, if possible

      case 118: M118(); break;                                    // M118: Display a message in the host console
      case 119: M119(); break;                                    // M119: Report endstop states
      case 120: M120(); break;                                    // M120: Enable endstops
      case 121: M121(); break;                                    // M121: Disable endstops

      #if ENABLED(TEMPERATURE_UNITS_SUPPORT)
        case 149: M149(); break;                                  // M149: Set temperature units
      #endif

      #if HAS_COLOR_LEDS
        case 150: M150(); break;                                  // M150: Set Status LED Color
      #endif

      case 201: M201(); break;                                    // M201: Set max acceleration for print moves (units/s^2)

      #if 0
        case 202: M202(); break;                                  // M202: Not used for Sprinter/grbl gen6
      #endif

      case 204: M204(); break;                                    // M204: Set acceleration
      case 205: M205(); break;                                    // M205: Set advanced settings

      #if HAS_SOFTWARE_ENDSTOPS
        case 211: M211(); break;                                  // M211: Enable, Disable, and/or Report software endstops
      #endif

      case 220: M220(); break;                                    // M220: Set Feedrate Percentage: S<percent> ("FR" on your LCD)

      #if ENABLED(DIRECT_PIN_CONTROL)
        case 226: M226(); break;                                  // M226: Wait until a pin reaches a state
      #endif

      #if ENABLED(BABYSTEPPING)
        case 290: M290(); break;                                  // M290: Babystepping
      #endif

      #if HAS_SOUND
        case 300: M300(); break;                                  // M300: Play beep tone
      #endif

      #if ENABLED(PHOTO_GCODE)
        case 240: M240(); break;                                  // M240: Trigger a camera
      #endif

      #if HAS_LCD_CONTRAST
        case 250: M250(); break;                                  // M250: Set LCD contrast
      #endif

      #if HAS_LCD_BRIGHTNESS
        case 256: M256(); break;                                  // M256: Set LCD brightness
      #endif

      #if ENABLED(EXPERIMENTAL_I2CBUS)
        case 260: M260(); break;                                  // M260: Send data to an i2c slave
        case 261: M261(); break;                                  // M261: Request data from an i2c slave
      #endif

      #if HAS_USER_THERMISTORS
        case 305: M305(); break;                                  // M305: Set user thermistor parameters
      #endif

      case 400: M400(); break;                                    // M400: Finish all moves

      #if HAS_BED_PROBE
        case 401: M401(); break;                                  // M401: Deploy probe
        case 402: M402(); break;                                  // M402: Stow probe
      #endif


      #if HAS_LEVELING
        case 420: M420(); break;                                  // M420: Enable/Disable Bed Leveling
      #endif

      #if HAS_MESH
        case 421: M421(); break;                                  // M421: Set a Mesh Bed Leveling Z coordinate
      #endif

      #if ENABLED(X_AXIS_TWIST_COMPENSATION)
        case 423: M423(); break;                                  // M423: Reset, modify, or report X-Twist Compensation data
      #endif

      #if ENABLED(BACKLASH_GCODE)
        case 425: M425(); break;                                  // M425: Tune backlash compensation
      #endif

      #if HAS_POWER_MONITOR
        case 430: M430(); break;                                  // M430: Read the system current (A), voltage (V), and power (W)
      #endif

      #if ENABLED(CONTROLLER_FAN_EDITABLE)
        case 710: M710(); break;                                  // M710: Set Controller Fan settings
      #endif

      #if ENABLED(GCODE_MACROS)
        case 810: case 811: case 812: case 813: case 814:
        case 815: case 816: case 817: case 818: case 819:
        M810_819(); break;                                        // M810-M819: Define/execute G-code macro
      #endif

      #if HAS_BED_PROBE
        case 851: M851(); break;                                  // M851: Set Z Probe Z Offset
      #endif

      #if ENABLED(SKEW_CORRECTION_GCODE)
        case 852: M852(); break;                                  // M852: Set Skew factors
      #endif

      #if ANY(HAS_MOTOR_CURRENT_SPI, HAS_MOTOR_CURRENT_PWM, HAS_MOTOR_CURRENT_I2C, HAS_MOTOR_CURRENT_DAC)
        case 907: M907(); break;                                  // M907: Set digital trimpot motor current using axis codes.
        #if EITHER(HAS_MOTOR_CURRENT_SPI, HAS_MOTOR_CURRENT_DAC)
          case 908: M908(); break;                                // M908: Control digital trimpot directly.
          #if HAS_MOTOR_CURRENT_DAC
            case 909: M909(); break;                              // M909: Print digipot/DAC current value
            case 910: M910(); break;                              // M910: Commit digipot/DAC value to external EEPROM
          #endif
        #endif
      #endif

      #if HAS_MICROSTEPS
        case 350: M350(); break;                                  // M350: Set microstepping mode. Warning: Steps per unit remains unchanged. S code sets stepping mode for all drivers.
        case 351: M351(); break;                                  // M351: Toggle MS1 MS2 pins directly, S# determines MS1 or MS2, X# sets the pin high/low.
      #endif

      #if ENABLED(DEBUG_GCODE_PARSER)
        case 800: parser.debug(); break;                          // M800: GCode Parser Test for M
      #endif

      #if ENABLED(I2C_POSITION_ENCODERS)
        case 860: M860(); break;                                  // M860: Report encoder module position
        case 861: M861(); break;                                  // M861: Report encoder module status
        case 862: M862(); break;                                  // M862: Perform axis test
        case 863: M863(); break;                                  // M863: Calibrate steps/mm
        case 864: M864(); break;                                  // M864: Change module address
        case 865: M865(); break;                                  // M865: Check module firmware version
        case 866: M866(); break;                                  // M866: Report axis error count
        case 867: M867(); break;                                  // M867: Toggle error correction
        case 868: M868(); break;                                  // M868: Set error correction threshold
        case 869: M869(); break;                                  // M869: Report axis error
      #endif

      #if ENABLED(Z_STEPPER_AUTO_ALIGN)
        case 422: M422(); break;                                  // M422: Set Z Stepper automatic alignment position using probe
      #endif

      #if ENABLED(TOUCH_SCREEN_CALIBRATION)
        case 995: M995(); break;                                  // M995: Touch screen calibration for TFT display
      #endif

      case 999: M999(); break;                                    // M999: Restart after being Stopped

      #if ENABLED(UBL_MESH_WIZARD)
        case 1004: M1004(); break;                                // M1004: UBL Mesh Wizard
      #endif

      #if ENABLED(MAX7219_GCODE)
        case 7219: M7219(); break;                                // M7219: Set LEDs, columns, and rows
      #endif

      #if ENABLED(HAS_MCP3426_ADC)
        case 3426: M3426(); break;                                // M3426: Read MCP3426 ADC (over i2c)
      #endif
      case 8888:
        SERIAL_ECHO("ok\n");
        delay(10);
        NVIC_SystemReset();
        break;

      default: parser.unknown_command_warning(); break;
    }
    break;

    #if ENABLED(MARLIN_DEV_MODE)
      case 'D': D(parser.codenum); break;                         // Dn: Debug codes
    #endif

    #if ENABLED(REALTIME_REPORTING_COMMANDS)
      case 'S': case 'P': case 'R': break;                        // Invalid S, P, R commands already filtered
    #endif

    default:
      parser.unknown_command_warning();
  }

  if (!no_ok) queue.ok_to_send();

  SERIAL_OUT(msgDone); // Call the msgDone serial hook to signal command processing done
}

#if ENABLED(M100_FREE_MEMORY_DUMPER)
  void M100_dump_routine(FSTR_P const title, const char * const start, const uintptr_t size);
#endif

/**
 * Process a single command and dispatch it to its handler
 * This is called from the main loop()
 */
void GcodeSuite::process_next_command() {
  GCodeQueue::CommandLine &command = queue.ring_buffer.peek_next_command();

  PORT_REDIRECT(SERIAL_PORTMASK(command.port));

  if (DEBUGGING(ECHO)) {
    SERIAL_ECHO_START();
    SERIAL_ECHOLN(command.buffer);
    #if ENABLED(M100_FREE_MEMORY_DUMPER)
      SERIAL_ECHOPGM("slot:", queue.ring_buffer.index_r);
      M100_dump_routine(F("   Command Queue:"), (const char*)&queue.ring_buffer, sizeof(queue.ring_buffer));
    #endif
  }

  // Parse the next command in the queue
  parser.parse(command.buffer);
  process_parsed_command();
}

#pragma GCC diagnostic push
#if GCC_VERSION >= 80000
  #pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

/**
 * Run a series of commands, bypassing the command queue to allow
 * G-code "macros" to be called from within other G-code handlers.
 */
void GcodeSuite::process_subcommands_now(FSTR_P fgcode) {
  PGM_P pgcode = FTOP(fgcode);
  char * const saved_cmd = parser.command_ptr;        // Save the parser state
  for (;;) {
    PGM_P const delim = strchr_P(pgcode, '\n');       // Get address of next newline
    const size_t len = delim ? delim - pgcode : strlen_P(pgcode); // Get the command length
    char cmd[len + 1];                                // Allocate a stack buffer
    strncpy_P(cmd, pgcode, len);                      // Copy the command to the stack
    cmd[len] = '\0';                                  // End with a nul
    parser.parse(cmd);                                // Parse the command
    process_parsed_command(true);                     // Process it (no "ok")
    if (!delim) break;                                // Last command?
    pgcode = delim + 1;                               // Get the next command
  }
  parser.parse(saved_cmd);                            // Restore the parser state
}

#pragma GCC diagnostic pop

void GcodeSuite::process_subcommands_now(char * gcode) {
  char * const saved_cmd = parser.command_ptr;        // Save the parser state
  for (;;) {
    char * const delim = strchr(gcode, '\n');         // Get address of next newline
    if (delim) *delim = '\0';                         // Replace with nul
    parser.parse(gcode);                              // Parse the current command
    process_parsed_command(true);                     // Process it (no "ok")
    if (!delim) break;                                // Last command?
    *delim = '\n';                                    // Put back the newline
    gcode = delim + 1;                                // Get the next command
  }
  parser.parse(saved_cmd);                            // Restore the parser state
}

#if ENABLED(HOST_KEEPALIVE_FEATURE)

  /**
   * Output a "busy" message at regular intervals
   * while the machine is not accepting commands.
   */
  void GcodeSuite::host_keepalive() {
    const millis_t ms = millis();
    static millis_t next_busy_signal_ms = 0;
    if (!autoreport_paused && host_keepalive_interval && busy_state != NOT_BUSY) {
      if (PENDING(ms, next_busy_signal_ms)) return;
      PORT_REDIRECT(SerialMask::All);
      switch (busy_state) {
        case IN_HANDLER:
        case IN_PROCESS:
          SERIAL_ECHO_MSG(STR_BUSY_PROCESSING);
          TERN_(FULL_REPORT_TO_HOST_FEATURE, report_current_position_moving());
          break;
        case PAUSED_FOR_USER:
          SERIAL_ECHO_MSG(STR_BUSY_PAUSED_FOR_USER);
          TERN_(FULL_REPORT_TO_HOST_FEATURE, set_and_report_grblstate(M_HOLD));
          break;
        case PAUSED_FOR_INPUT:
          SERIAL_ECHO_MSG(STR_BUSY_PAUSED_FOR_INPUT);
          TERN_(FULL_REPORT_TO_HOST_FEATURE, set_and_report_grblstate(M_HOLD));
          break;
        default:
          break;
      }
    }
    next_busy_signal_ms = ms + SEC_TO_MS(host_keepalive_interval);
  }

#endif // HOST_KEEPALIVE_FEATURE
