# A-axis hardware commissioning

This firmware repurposes the original extruder hardware as a fourth linear
axis:

- G-code axis: `A` (called `I` internally by Marlin)
- Stepper connector: original `E0` connector
- Minimum endstop connector: original filament / `MT_DET` connector
- Minimum endstop MCU pin: `PA15`
- Initial steps: `640 steps/mm`
- Initial maximum speed: `3 mm/s`
- Initial maximum acceleration: `5 mm/s²`
- Homing speed: `2 mm/s`, followed by a `1 mm/s` second approach
- Temporary software travel: `A=0..25 mm`

Positive A motion must move away from the minimum switch. Negative A motion
must move toward it.

The 25 mm maximum is only a conservative commissioning limit. Replace
`I_MAX_POS` in `Marlin/Configuration.h` with the measured usable stroke and
reflash after commissioning.

## Safety rules

1. Never connect, disconnect, or rearrange a stepper cable while the board is
   powered.
2. Keep a hand on a physical power switch during every first movement and
   homing test. A serial `M112` is useful, but removing motor power is the most
   reliable way to stop motion when wiring or direction is wrong.
3. Start with the carriage well away from both mechanical ends.
4. Do not rely on the minimum switch to stop ordinary `G0` / `G1` moves.
   Marlin normally monitors it as a stopping input during homing. Software
   endstops protect ordinary moves only after the axis has been homed.
5. Rehome after power cycling, manually turning the screw, loosening the belt,
   or disabling the A motor.
6. Keep all tests below the temporary 25 mm software maximum until the usable
   stroke has been measured and configured.

## 1. Check wiring with power disconnected

1. Switch off and unplug the controller.
2. Connect the ballscrew motor to the original `E0` motor connector. Do not
   use `E1`, `Z2`, or an endstop connector.
3. If using the original E motor cable, keep its conductor order unchanged.
4. If making a new cable, identify the two motor coils with a multimeter:
   conductors belonging to one coil show a low resistance to each other;
   conductors from different coils read open. Each coil pair must occupy one
   driver coil pair. An incorrectly paired cable usually causes buzzing,
   vibration, or erratic movement instead of rotation.
5. Connect the sensor to the original filament-runout / `MT_DET` connector.
   Preserve the original signal, ground, and supply order. The firmware reads
   its signal on `PA15`.
6. With the controller still off, turn the ballscrew by hand through the
   intended test region. Correct binding, pulley misalignment, belt rubbing,
   or excessive belt tension before powered tests.
7. Put the carriage near the middle of its available travel, at least 10 mm
   away from the minimum switch and comfortably clear of the far end.
8. Mark the motor pulley or screw so that one complete revolution can be
   recognized.

## 2. Establish the serial session and reset EEPROM

Connect at `115200` baud and send:

```gcode
M115
```

Wait for a firmware identification response and `ok`. Then load the compiled
defaults and overwrite any saved extruder settings from the previous
firmware:

```gcode
M502
M500
M503
```

In the `M503` report, confirm approximately:

```text
M92 ... A640.00
M203 ... A3.00
M201 ... A5
```

Do not continue if the serial connection is unreliable or the A values are
missing.

## 3. Prove the minimum sensor before moving the motor

Release the sensor and send:

```gcode
M119
```

The response must include:

```text
a_min: open
```

Hold the sensor in its mechanically triggered state and send `M119` again.
It must now include:

```text
a_min: TRIGGERED
```

Repeat this at least ten times while gently moving the sensor cable and
connector. Every reading must be unambiguous and repeatable.

If the state is always the same:

1. Power off.
2. Confirm the connector is the filament / `MT_DET` connector.
3. Check connector orientation and continuity.
4. Check that the sensor signal changes between its released and actuated
   states.

If the two states are exactly reversed, change:

```cpp
#define I_MIN_ENDSTOP_INVERTING false
```

to `true`, rebuild, flash, and repeat this section. Do not attempt homing with
reversed or intermittent sensor reporting.

## 4. Prove unloaded motor rotation

For the first motor test, disconnect the belt or loosen the coupling so the
motor cannot drive the carriage. Power must be off while changing the
mechanical connection.

After restoring power and reconnecting serial, send:

```gcode
G91
G1 A5 F30
M400
G90
```

`F30` is only `0.5 mm/s`. With the initial 640 steps/mm calibration, an A5
command produces half a motor revolution.

Expected result:

- The shaft turns smoothly by about half a revolution.
- It does not buzz, jump between positions, or repeatedly stall.
- The driver and motor do not heat noticeably during this short test.

If it only buzzes or moves erratically, immediately remove power and recheck
the two coil pairs. Do not swap motor wires while powered.

## 5. Prove direction with the mechanism connected

Power off, reconnect the belt or coupling, and place the carriage at least
10 mm away from the minimum switch.

After powering on, send a single small positive move:

```gcode
G91
G1 A1 F30
M400
G90
```

The carriage must move **away from the A-min sensor**.

If it moves toward the sensor, remove power and change:

```cpp
#define INVERT_I_DIR false
```

to:

```cpp
#define INVERT_I_DIR true
```

Rebuild, flash, reset EEPROM with `M502` and `M500`, and repeat the 1 mm test.
Do not reverse the motor connector as a first choice; keeping direction
defined in firmware makes the wiring and documentation unambiguous.

## 6. Calibrate travel distance

Do this before homing. Start with the carriage clear of both ends and measure
its starting position with a caliper, ruler, or dial indicator. Always
approach the measurement in the same direction so belt or screw backlash does
not affect the result.

Send:

```gcode
G91
G1 A10 F60
M400
G90
```

`F60` is `1 mm/s`. Measure the actual carriage movement.

Calculate:

```text
corrected_steps_per_mm = current_steps_per_mm × commanded_distance / measured_distance
```

For example, if A10 moves only 5.00 mm:

```text
640 × 10 / 5 = 1280 steps/mm
```

Apply a trial correction without reflashing:

```gcode
M92 A1280
M500
```

Repeat the 10 mm measurement until the error is acceptable. Then put the
confirmed value into `DEFAULT_AXIS_STEPS_PER_UNIT` in
`Marlin/Configuration.h`.

With a 1.8° motor, 32 microsteps, 1:1 belt drive, 5 mm thread pitch, and two
starts, one screw revolution should move 10 mm and the correct value is
640 steps/mm. If one revolution moves only 5 mm, the screw specification was
giving a 5 mm lead and the expected value is 1280 steps/mm.

## 7. Perform the first A homing cycle

Only continue after:

- `M119` reliably reports `a_min: open` and `TRIGGERED`.
- Positive A motion has been proven to move away from the sensor.
- The measured travel scale is reasonably correct.

Place the carriage 5 to 10 mm away from A-min and send:

```gcode
M119
G28 A
```

Expected sequence:

1. The carriage moves toward A-min at 2 mm/s.
2. It stops when the sensor triggers.
3. It backs away by 1 mm.
4. It approaches again at 1 mm/s.
5. It stops and establishes A=0.

Immediately remove power if it moves away from the switch, reaches the switch
without stopping, or makes an unexpected fast move.

After a successful home:

```gcode
M114
```

The A coordinate should be zero. Check repeatability at least five times:

```gcode
G1 A5 F60
G28 A
```

The carriage should return to the same physical zero without grinding or
creeping farther into the switch.

## 8. Measure and configure the usable stroke

The compiled maximum is temporarily 25 mm. Do not disable software endstops
to work around it.

1. Home A.
2. Power off.
3. Carefully move or turn the mechanism by hand from the repeatable A-min
   location toward the far mechanical limit.
4. Measure the available distance from the homed switch position.
5. Subtract a safety margin appropriate to the mechanism, normally at least
   5 mm.
6. Set that result as `I_MAX_POS` in `Marlin/Configuration.h`.
7. Rebuild and flash.
8. Send `M502`, then `M500`.
9. Send `M211` and confirm software endstops are enabled.

Do not configure the raw screw length. Use the carriage's actual collision-free
stroke, including pulley, bearing, nut, and switch geometry.

## 9. Test the loaded mechanism progressively

Home first, then test increasing speeds without approaching either end:

```gcode
G28 A
G1 A5  F30
G1 A10 F60
G1 A15 F120
G1 A20 F180
```

The feed rates correspond to 0.5, 1, 2, and 3 mm/s.

Listen and watch for:

- missed steps or a sudden position jump;
- periodic binding once per screw or pulley revolution;
- belt tooth engagement problems;
- excessive belt tension;
- motor or driver overheating;
- vibration during starting or stopping.

The TMC driver is in standalone mode, so motor current is not set with
`M906`. Do not change the board's current adjustment unless the mechanism
stalls or overheats and the board's current-setting procedure is known.

For an even gentler acceleration test:

```gcode
M201 A2
M500
```

Restore the compiled value with:

```gcode
M201 A5
M500
```

The maximum speed, acceleration, steps, and classic jerk can be inspected or
tuned over serial:

```gcode
M92
M203
M201
M205
```

Save validated changes with `M500` and also copy final calibration values back
into the source configuration so a future `M502` restores the correct values.

## Acceptance checklist

- [ ] Motor is connected to E0 and never plugged/unplugged while powered.
- [ ] Sensor is connected to the filament / MT_DET connector.
- [ ] `M119` is repeatable: released=`open`, actuated=`TRIGGERED`.
- [ ] Positive A moves away from A-min.
- [ ] A10 produces 10.00 mm of physical motion within the required tolerance.
- [ ] `G28 A` performs both approaches and returns repeatably to A=0.
- [ ] `I_MAX_POS` is the measured collision-free stroke minus a safety margin.
- [ ] Motion is smooth at 0.5, 1, 2, and 3 mm/s under load.
- [ ] No missed steps, binding, unusual heating, or belt tracking problems.
- [ ] Final steps, limits, speed, and acceleration are saved in EEPROM and in
      the source configuration.
