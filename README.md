# Dual-Axis Solar Tracker (ProDex IoT Task 2)

An Arduino Uno based solar tracker simulated in TinkerCAD. Four LDRs measure light from four directions, and two servos turn and tilt the panel toward the brighter side.

**TinkerCAD simulation:** PASTE_YOUR_LINK_HERE (set to "anyone with the link can view")

## Contents

| File | Description |
|---|---|
| `solar_tracker.ino` | Arduino source code |
| `images/` | Circuit screenshots, schematic and Serial Monitor output |
| `README.md` | This write-up |

## Components

| Part | Qty | Notes |
|---|---|---|
| Arduino Uno R3 | 1 | |
| Photoresistor (LDR) | 4 | TL, TR, BL, BR positions |
| 10 kΩ resistor | 4 | One per LDR (voltage divider) |
| Micro servo | 2 | Horizontal and vertical axis |
| LED (green) + 220 Ω resistor | 1 | "Locked on light" indicator |
| Breadboard | 1 | |

## Wiring

| Connection | Pin |
|---|---|
| LDR top-left divider junction | A0 |
| LDR top-right divider junction | A1 |
| LDR bottom-left divider junction | A2 |
| LDR bottom-right divider junction | A3 |
| Horizontal servo signal | D9 |
| Vertical servo signal | D10 |
| Lock indicator LED (via 220 Ω) | D7 |

Each LDR is wired as a voltage divider: one LDR leg to 5V, the other leg to the analog pin and to a 10 kΩ resistor that goes to GND. More light means lower LDR resistance, a higher voltage at the junction and a higher `analogRead()` value. Servo power wires go to the 5V and GND rails.

Screenshots: `images/circuit.png` (breadboard view) and `images/schematic.png` (schematic view).

## How it works

1. Each LDR is read five times and averaged to reduce noise.
2. The code averages the pairs: top vs bottom gives the vertical error, left vs right gives the horizontal error.
3. If an error is larger than `THRESHOLD`, the matching servo moves one degree toward the brighter side. If both errors are inside the threshold, nothing moves and the LED lights (stable tracking).
4. Servo angles are limited with `constrain()` so the panel cannot hit its mechanical stops.

## Edge cases handled

| Case | Behaviour |
|---|---|
| Sensor noise | Five-sample averaging per LDR |
| Tiny light differences | Dead-band (`THRESHOLD = 30`) stops jitter and constant hunting |
| Night / very low light | When the average falls below `DARK_LEVEL`, tracking pauses and the panel holds position |
| Prolonged darkness | After 5 seconds of darkness the panel glides back to the home position (90°, 90°), ready for sunrise |
| Light returns | Tracking resumes automatically and the dark timer resets |
| Faulty or disconnected LDR | A reading stuck at the extremes puts the system into a safe hold instead of steering on bad data |
| Servo travel limits | Horizontal 10° to 170°, vertical 30° to 150° |
| Wrong mounting direction | `H_DIR` and `V_DIR` can be flipped to -1 without rewriting the logic |

## Additional functionality

- **Lock-on LED:** lights when the panel is aligned and within the threshold.
- **Night return to home:** the panel parks itself at the home position after sustained darkness.
- **Live telemetry:** the Serial Monitor prints all four LDR values, both servo angles and the state (`tracking`, `locked`, dark states).

## Output evidence

| State | Screenshot |
|---|---|
| Too dark, holding position | `images/serial_dark.png` |
| Night, returning to home | `images/serial_night_home.png` |
| Equal light, locked | `images/serial_locked.png` |
| One LDR brighter, tracking (angles changing) | `images/serial_tracking.png` |

Example tracking output with the top-left LDR bright:

```
TL:966 TR:54 BL:54 BR:54 | H:124 V:124 | tracking
TL:966 TR:54 BL:54 BR:54 | H:125 V:125 | tracking
```

**Simulation note:** in TinkerCAD the LDRs are not physically attached to the servos, so turning the servos does not change the light on the LDRs. Under a constant imbalance the servos keep stepping until they reach their angle limits. On a real panel, the imbalance shrinks as the panel turns toward the light and the system settles on its own.

## Adaptation to real-world problems

**Dust, dirt and shading.** LDRs sit outdoors and their sensitivity drifts as they get dirty or age, and one dusty sensor skews the whole comparison. Real systems mount the sensors under a shared diffuser, keep them matched, and recalibrate periodically. The dead-band here is a software version of the same idea: small differences are ignored rather than chased.

**Clouds and weather.** Under heavy cloud the light is diffuse and nearly equal from all directions, so a naive tracker wanders. The threshold and the dark hold prevent this. A production system would also filter readings over a longer window so passing clouds or a bird's shadow do not trigger movement.

**Wind and mechanical load.** Real panels are heavy and catch wind. Hobby servos cannot hold them, so a real build needs geared motors or linear actuators, limit switches at the ends of travel, and a stow position in high wind. The software angle limits here stand in for those end stops.

**Power budget.** Servos draw a lot of current, especially when stalled, and cannot be powered from the Arduino's 5V pin on real hardware. They need a separate supply with a common ground. Tracking continuously also costs energy, so real trackers move in small steps every few minutes rather than every loop. A tracker that wastes more energy moving than it gains from alignment is a net loss, so the step interval is a design trade-off.

**Night and sunrise.** The night return-to-home behaviour mirrors real trackers, which park facing east so that they are already aligned when the sun rises.

**Hybrid control.** Sensor-only tracking can lose the sun on overcast days. Commercial systems combine an astronomical model (sun position from date, time and GPS) with sensor feedback for fine correction, which keeps tracking correctly even when the light signal is weak.

**Reliability.** Sensors fail, wires come loose and readings go out of range. Checking for stuck values and falling back to a safe state prevents a single failed LDR from sending the panel to a wrong position.

## Further applications

- **Solar farms and rooftop panels:** higher energy yield than fixed panels, typically a sizeable gain depending on latitude and season.
- **Solar concentrators and cookers:** keeping a reflector pointed at the sun.
- **Satellite dishes and antennas:** directional alignment using signal strength instead of light.
- **Heliostats and daylighting systems:** reflecting sunlight into buildings.
- **Light-seeking robots and sensor nodes:** the same four-sensor comparison steers a robot toward a light source.
- **Greenhouse and agriculture:** orienting reflectors or shade systems.

## How to run

1. Open the TinkerCAD link above.
2. Click **Start Simulation** and open the Serial Monitor.
3. Click an LDR and use its slider to change the light level. The servos move toward the brighter side.
4. Set all four LDRs to minimum for 5 seconds to see the night return.
