# Smart Kitchen Implementation Step-By-Step

This guide is written for a beginner. It explains what to do, why to do it, how it works, when it starts, how to wire it, power needs, and damage risks.

## 1. What This Project Is and Why It Matters

Project goal:
- Build a stationary smart kitchen safety system that can detect dangerous conditions and respond automatically.

Where it helps:
- Home kitchens where gas leakage, overheating, or unattended flame can become dangerous.
- Small hostels or shared flats where cooking is frequent and users may forget to turn off gas.

Why this project is important:
- A simple one-threshold alarm gives many false alarms.
- This project uses multi-condition logic, so it behaves closer to real kitchen conditions.
- It can trigger ventilation and gas cutoff, not only sound an alarm.

When system starts:
- It starts immediately after power is applied.
- It enters a startup safety mode first, then normal monitoring.

## 2. Final Feature Lock (Keep This Fixed)

Do not change these core requirements:
- Multi-condition logic (not single threshold)
- False alarm prevention
- DHT22 (not DHT11)
- Gas cutoff mechanism
- Stop alarm button

System states:
- NORMAL
- COOKING_NORMAL
- WARNING
- ALARM_ACTIVE
- CUTOFF_LOCKED

## 3. Safety Rules Before You Touch Hardware

1. Work with low-voltage DC only during development.
2. If you are not trained for AC mains wiring, do not use AC fan wiring directly.
3. Prefer a 12V DC fan to avoid mains risk.
4. Disconnect power before changing wires.
5. Keep a fuse on the 12V input line.
6. Keep a common ground between 12V supply and 5V logic system.
7. Keep wires insulated and strain-relieved.
8. For a short 10 to 20 minute demo, do not build a permanent raw series pack from loose 18650 cells.
9. Do not use a voltage divider as a power regulator (divider is only for voltage sensing).
10. Do not feed servo power from Arduino onboard regulator under load.

## 4. Complete Hardware List With Purpose, Power, and Risks

Note: currents vary by vendor. Check your module labels and datasheets.

| Item | Why needed | What it does | Voltage | Typical current | Main connection | Damage risk | Protection |
|---|---|---|---|---|---|---|---|
| Arduino Uno | Main controller | Runs logic and controls outputs | 5V | 50 to 80mA | USB or regulated 5V | Overvoltage on pins | Never feed sensor pin above 5V |
| MQ-2 module | Gas/smoke trend | Detects combustible gas concentration | 5V | 150 to 200mA (heater) | VCC, GND, A0 | Wrong polarity, heater stress | Correct polarity, stable 5V |
| DHT22 | Better temp/humidity sensing than DHT11 | Temperature and humidity for risk engine | 3.3 to 6V | 1 to 2.5mA | VCC, GND, DATA + pull-up | Static and bad wiring | 10k pull-up, short wires |
| Flame sensor module | Flame indicator | Adds fire context to logic | 3.3 to 5V | 15 to 20mA | VCC, GND, DO | Sunlight/noise false trigger | Shielding and persistence timer |
| LCD 16x2 I2C | User feedback | Shows state, sensor values, alerts | 5V | 20 to 30mA | SDA, SCL, VCC, GND | Wrong I2C address/wiring | I2C scanner test first |
| Buzzer (active) | Audible alarm | Warns user during danger | 5V | 20 to 35mA | Digital pin via transistor if needed | Driving too much current from pin | Use transistor if loud buzzer |
| MG90S servo (recommended) | Vent flap actuation | Opens/closes flap for airflow | 4.8 to 6V | 100mA idle, up to 700mA peak | Signal + separate 5V + GND | Stall current, gear wear | Separate 5V rail, avoid hard stops |
| SG90 servo (optional only) | Lightweight flap only | Lower torque option | 4.8 to 6V | Similar but weaker torque | Same as MG90S | Plastic gear wear, stalling | Use only for very light flap |
| Servo bracket and linkage kit | Mechanical coupling | Transfers servo motion to vent flap reliably | N/A | N/A | Servo horn, mount, pushrod/link | Jamming or slippage without rigid mount | Use one-axis MG90S mount for flap; use two-axis pan-tilt only if truly needed |
| 2-channel relay module | Switches fan and valve | Isolates controller from load side | 5V coil | ~70mA per channel | IN pins + COM/NO/NC contacts | Contact damage from high load | Stay within relay rating |
| 12V DC exhaust fan | Ventilation | Removes gas/smoke from enclosure | 12V | 200 to 700mA | Through relay contact | Motor inrush, stuck blades | Fuse + free airflow |
| 12V NC solenoid gas valve | Automatic gas cutoff | Closes gas path in danger state | 12V | 300mA to 1A | Through relay contact | Coil heating if misused | Correct duty and rated supply |
| Buck converter 12V to 5V | Stable logic power | Powers Uno, sensors, servo | 12V input, 5V output | 2A to 3A output recommended | Between 12V supply and 5V bus | Wrong trim voltage | Set to 5.00V before connecting |
| 18650 cells in holders (demo-only source) | Optional temporary source | Can be used for short demo logic-side power through regulated modules | 3.7V nominal per cell (4.2V full) | Depends on cell and module | Use through power bank board or regulated converter, not direct to loads | Over-discharge, imbalance, wiring heat if used as raw series pack | Use matched cells, monitor voltage, and recharge cells individually |
| 5V USB power bank (fallback logic source) | Easy beginner option | Powers Arduino logic side when no buck is available | 5V | Depends on bank (2A or higher preferred) | Arduino USB input | Auto-off at low load, unstable for servo peaks | Test runtime and keep shared ground with control side |
| Stop button | Human acknowledgement | Mute alarm safely without disabling protection | 5V logic level | Very low | Digital input with pull-up | Bounce/noise | Software debounce |
| Optional reset button | Controlled re-arm | Manual reset after safe window | 5V logic level | Very low | Digital input with pull-up | Accidental press | Long-press requirement |
| 12V adapter | Main source | Powers load and buck converter | 12V | 3A minimum, 5A better | Main power input | Undersized supply heats/drops voltage | Choose quality adapter |
| Distribution board / terminal block | Clean power branching | Safe multi-line distribution | N/A | N/A | Between source and branches | Loose contacts | Screw lock and labeling |

## 5. Power Budget and Why It Matters

Why this section matters:
- Undersized power causes random resets, false alarms, servo jitter, and relay chatter.

Basic formulas:
- Power: P = V x I
- Current required: I = P / V

Example estimate (replace with your actual labels):
- 12V fan: 0.40A
- 12V valve: 0.50A
- 5V side (Uno + sensors + LCD + servo peaks averaged): 1.20A equivalent from buck

12V adapter current target:
- Load side direct = 0.40A + 0.50A = 0.90A
- Buck input equivalent for 5V side approx = (5V x 1.20A) / (12V x 0.85 efficiency) approx 0.59A
- Total approx = 1.49A
- Use at least 3A for margin. 5A gives better reliability.

If you ever connect 4x 18650 Li-ion cells in series (future advanced version):
- This is optional and not required for your 10 to 20 minute demo.
- Nominal pack voltage is about 14.8V.
- Full-charge pack voltage is about 16.8V.
- This is not a direct 12V source.
- 12V fan and 12V valve should receive regulated 12V for long-term reliability.

Critical power rules:
1. Do not power servo from Arduino 5V pin when under load.
2. Use a regulated 5V rail (buck converter preferred) for servo and logic.
3. Keep all grounds common.
4. Add a fuse at 12V input.
5. Never use a voltage divider to power loads.

### 5.1 Easy Fallback Setup (If You Cannot Buy BMS or 12V Buck Now)

Use this temporary setup for prototype and demo:
1. Power Arduino and sensors from a good 5V USB power bank.
2. Use a separate 12V adapter for fan and valve through relay contacts.
3. Keep Arduino ground and relay input-side ground common.
4. If 12V hardware is unavailable, keep fan/valve as simulated outputs (LED/buzzer) and demonstrate full logic flow.
5. If you want to use your 18650 cells, use them only through a regulated 5V source (power bank board/case) for logic-side demo power.

Important note:
- Avoid raw series wiring of loose 18650 cells for direct fan/valve powering in beginner demos.

## 6. Wiring Map (Recommended Pin Plan)

Use this map unless you have a reason to change.

Digital and analog pins:
- A0: MQ-2 analog output
- D2: Stop button (use INPUT_PULLUP)
- D3: Flame sensor digital output
- D4: Buzzer control
- D5: Relay channel 1 (fan)
- D6: Relay channel 2 (gas valve)
- D7: Yellow LED (warning)
- D8: Red LED (alarm)
- D9: Green LED (normal)
- D10: Servo signal
- D11: Optional reset button (INPUT_PULLUP)
- D12: DHT22 data
- A4 (SDA), A5 (SCL): LCD I2C

Power rails:
- Regulated 12V rail: fan and valve through relay contacts
- 5V logic rail from buck or USB power bank: Arduino, sensors, LCD, relay module coil side, buzzer
- Servo 5V rail: use a strong regulated 5V source (same buck or separate 5V module)
- Common GND between all modules

## 7. Exact Connection Steps (No Skipping)

### 7.1 Build power lines first
Choose one of these two methods.

Method A (recommended final wiring):
1. Connect regulated 12V source positive to fuse input.
2. Fuse output to distribution board 12V positive line.
3. 12V source negative to distribution board ground line.
4. Connect buck converter input to 12V and ground.
5. Adjust buck output to exact 5.00V before connecting electronics.
6. Build a 5V distribution line from buck output.

Method B (easy fallback with minimal purchases):
1. Use USB power bank to power Arduino through USB.
2. Use separate 12V adapter for fan and valve load line.
3. Connect relay module VCC/GND to Arduino-side 5V/GND.
4. Connect Arduino GND to the relay input-side GND (common control ground).
5. Keep relay contact side switching the separate 12V adapter to fan and valve.

### 7.2 Connect Arduino and sensors
1. If using buck 5V rail: Arduino 5V to 5V rail, Arduino GND to ground rail.
2. If using power bank via USB: keep USB for Arduino power and still connect Arduino GND to system control ground.
3. MQ-2: VCC to 5V, GND to GND, A0 to Arduino A0.
4. DHT22:
- Pin 1 VCC to 5V
- Pin 2 DATA to D12
- Pin 3 NC
- Pin 4 GND to GND
- 10k resistor between Pin 1 and Pin 2
5. Flame sensor: VCC to 5V, GND to GND, DO to D3.
6. LCD I2C: VCC to 5V, GND to GND, SDA to A4, SCL to A5.

### 7.3 Connect user interface parts
1. Green LED: anode to D9 through 220 ohm resistor, cathode to GND.
2. Yellow LED: anode to D7 through 220 ohm resistor, cathode to GND.
3. Red LED: anode to D8 through 220 ohm resistor, cathode to GND.
4. Buzzer: signal to D4 (or transistor driver if buzzer is high current), GND to GND.
5. Stop button:
- One side to D2
- Other side to GND
- Configure D2 as INPUT_PULLUP
6. Optional reset button same method on D11.

### 7.4 Connect relay and load side
1. Relay module VCC to 5V, GND to GND.
2. Relay IN1 to D5 (fan control), IN2 to D6 (valve control).
3. Fan switching via relay contact:
- 12V positive to relay COM1
- Relay NO1 to fan positive
- Fan negative to 12V ground
4. Valve switching via relay contact:
- 12V positive to relay COM2
- Relay NO2 to valve positive
- Valve negative to 12V ground

Note:
- Some relay modules are active LOW (ON when pin goes LOW). Check this in early testing.

### 7.5 Connect servo safely
1. Servo signal wire to D10.
2. Servo VCC to strong regulated 5V rail (buck preferred; avoid weak USB outputs).
3. Servo GND to common ground.
4. Do not power servo from Arduino onboard regulator.
5. Add a 470uF to 1000uF capacitor near servo 5V and GND to reduce reset/jitter problems.

## 8. Mechanical Assembly Step-By-Step

1. Build or choose a fixed frame. This project is stationary, no wheel movement required.
2. Mount fan near vent outlet so airflow exits kitchen zone.
3. Install servo mount and linkage first, then attach flap with free rotation and no hard mechanical jam.
4. For vent flap use one-axis bracket linkage with MG90S for better torque. Use two-axis pan-tilt only if your design really needs pan and tilt.
5. Place MQ-2 where gas concentration can be sensed but avoid direct flame heat blast.
6. Place DHT22 away from direct burner flame so it measures ambient change, not direct fire contact.
7. Place flame sensor in a line of sight region but shield from direct sunlight reflections.
8. Keep relay and power wiring away from sensitive sensor lines.
9. Tie and label all wires: PWR12, PWR5, GND, SIG.

Why this assembly style helps:
- Better sensor stability
- Less electrical noise
- Lower false alarms
- Easier troubleshooting later

## 9. Software Setup From Zero

1. Install Arduino IDE.
2. Select board: Arduino Uno.
3. Select correct COM port.
4. Install libraries:
- DHT sensor library
- Adafruit Unified Sensor (dependency)
- Servo
- LiquidCrystal_I2C
5. Create a new sketch and save project folder.

## 10. Firmware Architecture You Should Implement

Create one main loop that calls these functions in order:
1. readSensors
2. applyFilters
3. computeRiskScore
4. updateStateMachine
5. handleButtons
6. controlActuators
7. updateDisplayAndLogs

Why this architecture:
- Easy to debug
- Clear separation between sensing and actions
- Easy to tune thresholds later

## 11. Multi-Condition Logic (How It Works)

Do not use one threshold only.

### 11.1 Raw signals
- Gas level from MQ-2 analog
- Gas rise speed over time
- Temperature from DHT22
- Temperature rise speed
- Flame signal persistence

### 11.2 Example risk points
- Gas above warning threshold: +3
- Gas rising fast: +2
- Temperature above warning threshold: +2
- Temperature rising fast: +1
- Flame present unexpectedly: +2

Total risk score range: 0 to 10

### 11.3 State decision example
- 0 to 3: NORMAL or COOKING_NORMAL
- 4 to 6: WARNING
- 7 to 10: ALARM_ACTIVE
- ALARM_ACTIVE persisting beyond cutoff timer: CUTOFF_LOCKED

Why this helps:
- Normal cooking often raises one sensor temporarily.
- True danger usually affects multiple signals and persists.

## 12. False Alarm Prevention (Detailed)

Use all of the following together:

1. Startup warm-up lockout
- Ignore MQ-2 decision logic for first 90 to 120 seconds.

2. Moving average filter
- Keep last 10 samples for gas and temperature.
- Use average instead of single reading.

3. Outlier rejection
- If one sample jumps too far and returns immediately, ignore that sample.

4. Persistence timer
- Condition must remain true for N seconds before changing state.
- Example: warning needs 8 seconds, alarm needs 12 seconds.

5. Hysteresis
- Entry threshold higher than exit threshold.
- Example gas warning enter at 450, exit at 390.

6. Cross-sensor validation
- Gas high alone for a short burst is not enough.
- Gas high + rising temperature + flame persistence increases confidence.

## 13. DHT22 (Not DHT11) Integration Details

Why DHT22:
- Better accuracy and wider temperature range.
- Better stability for safety logic.

How it is used:
1. Read every 1 second.
2. Reject NaN or invalid values.
3. Track current temperature and rise rate.
4. Feed temperature features into risk score.

Damage risks for DHT22:
- Condensation and water droplets
- Long cable noise
- Static during wiring

Protection:
- Keep dry
- Keep cable short or twisted pair if needed
- Add basic enclosure venting but avoid direct steam

## 14. Gas Cutoff Mechanism (How and When It Triggers)

What it does:
- Closes gas path using solenoid valve when confirmed danger persists.

When it starts:
- In ALARM_ACTIVE, start cutoff confirmation timer.
- If danger remains beyond timer (example 10 to 20 seconds), go CUTOFF_LOCKED and energize valve control logic as per valve type.

How it works:
1. Relay switches power path to valve.
2. Valve changes state to close gas.
3. Fan remains ON, flap remains OPEN.
4. Buzzer and red LED indicate danger.

Important valve note:
- Valve behavior depends on type (normally closed vs normally open, energized-to-open vs energized-to-close).
- Confirm exact behavior from datasheet before final logic.

Damage risks:
- Continuous overvoltage heats valve coil.
- Wrong relay contact wiring can keep valve always energized.

Protection:
- Correct rated voltage
- Correct relay contact choice
- Temperature check of coil during long tests

## 15. Stop Alarm Button Logic (Mandatory)

Goal:
- Let user silence sound without removing safety actions.

Behavior:
1. Short press in ALARM_ACTIVE:
- Mute buzzer for fixed window (example 120 seconds)
- Keep fan ON
- Keep flap OPEN
- Keep cutoff active if already triggered

2. Long press (example 3 seconds):
- Allowed only when safe window is satisfied
- Clears alarm state and permits controlled return to NORMAL

Why this design helps:
- Prevents panic from loud buzzer
- Avoids unsafe user action that disables protection completely

## 16. Runtime Timeline (From Power-On to Incident)

1. Power ON
- Initialize pins, LCD, and serial logging
- Set safe default outputs (fan off, buzzer off, valve normal state, flap safe angle)

2. Warm-up mode
- MQ-2 lockout active
- Display WARMING UP

3. Monitoring mode
- Read sensors periodically
- Update moving averages
- Compute risk score
- Decide state and actuate outputs

4. Warning escalation
- If medium risk persists, enter WARNING
- Yellow LED and display warning

5. Alarm escalation
- If high risk persists, enter ALARM_ACTIVE
- Red LED, buzzer, fan, flap open

6. Cutoff lock
- If danger persists beyond cutoff timer, close gas and enter CUTOFF_LOCKED

7. Recovery path
- Require continuous safe readings for configured safe window
- Require long press reset before returning to normal

## 17. Suggested Default Thresholds (Tune Later)

These are starter values only:
- Temperature warning: 38C
- Temperature alarm: 45C
- Temperature rise warning: 1.2C per minute
- Temperature rise alarm: 2.5C per minute
- Gas warning threshold: baseline + 120 ADC counts
- Gas alarm threshold: baseline + 220 ADC counts
- Flame persistence: 1.5 seconds
- Warning persistence: 8 seconds
- Alarm persistence: 12 seconds
- Cutoff persistence after alarm: 10 seconds
- Safe recovery window: 60 seconds
- Buzzer mute window: 120 seconds

## 18. Output Action Table by State

| State | Fan | Flap servo | Buzzer | Gas valve | LEDs | LCD |
|---|---|---|---|---|---|---|
| NORMAL | OFF | Closed or standby | OFF | Normal | Green ON | Normal status |
| COOKING_NORMAL | Low or OFF | Slight open optional | OFF | Normal | Green blink | Cooking normal |
| WARNING | ON optional | Mid open | Beep pattern optional | Normal | Yellow ON | Warning message |
| ALARM_ACTIVE | ON | Full open | ON unless muted | Prepare cutoff timer | Red ON | Alarm active |
| CUTOFF_LOCKED | ON | Full open | ON unless muted | Closed (locked) | Red flash | Gas cutoff locked |

## 19. Full Build and Test Workflow (Detailed Execution)

### Phase A: Individual module testing
1. Test Uno blink.
2. Test DHT22 reading stability for 10 minutes.
3. Test MQ-2 baseline after warm-up.
4. Test flame sensor trigger with controlled source.
5. Test LEDs and buzzer.
6. Test servo sweep without flap load.
7. Test relay clicks and load switching with fan only.
8. Test valve switching with relay.

### Phase B: Integration without cutoff active
1. Integrate sensors and display.
2. Implement risk scoring and states.
3. Keep valve logic disabled first.
4. Validate normal and warning transitions.

### Phase C: Integration with cutoff
1. Enable valve control after logic is stable.
2. Verify cutoff only under sustained danger.
3. Verify cutoff lock does not clear on short press.

### Phase D: Long run validation
1. Run 30 to 60 minute normal cooking simulation.
2. Confirm no unnecessary alarm.
3. Inject controlled abnormal conditions.
4. Verify alarm and cutoff timings.

## 20. Damage and Failure Scenarios You Must Plan For

1. Servo stalling at mechanical hard stop
- Symptom: buzzing servo, hot casing, jitter
- Fix: adjust flap linkage, limit servo angles

2. Relay chatter due to weak supply
- Symptom: rapid clicking, unstable load
- Fix: stronger power supply, separate load and logic decoupling

3. MQ-2 contamination or aging
- Symptom: drifting baseline, too many warnings
- Fix: periodic recalibration and replacement plan

4. DHT22 invalid data
- Symptom: NaN or sudden impossible values
- Fix: retry read, ignore invalid sample, inspect pull-up resistor

5. Valve not responding
- Symptom: no click and no flow change
- Fix: verify coil voltage and relay wiring, check fuse

6. Ground mismatch
- Symptom: random resets and noisy readings
- Fix: enforce single common ground network

7. Reversed polarity damage
- Symptom: module heats immediately
- Fix: power off instantly, inspect before re-powering

## 21. Recommended Maintenance Plan

Weekly:
- Check wire tightness and connector heat marks
- Clean dust near fan intake

Monthly:
- Re-check MQ-2 baseline in clean air
- Test stop button short and long press behavior
- Test one alarm-to-cutoff scenario safely

Before demo day:
- Re-verify all threshold values
- Re-test all state transitions
- Keep spare relay module and spare servo horn

## 22. Beginner Troubleshooting Guide

Problem: LCD shows nothing
- Check I2C address
- Check SDA/SCL wiring
- Adjust LCD contrast potentiometer

Problem: Servo jitters continuously
- Use separate stable 5V rail
- Add common ground
- Keep servo wires away from relay/load wires

Problem: Frequent false warning
- Increase persistence time
- Increase hysteresis gap
- Recalibrate MQ-2 baseline

Problem: Alarm never triggers
- Thresholds too high
- Sensor wiring wrong
- Risk scoring points too conservative

Problem: Cutoff triggers too quickly
- Alarm persistence timer too short
- Over-weighted single sensor condition

## 23. Demo Script You Can Follow

Scenario 1: Normal cooking profile
1. Show system in NORMAL.
2. Simulate mild rise to COOKING_NORMAL.
3. Show no alarm and no cutoff.

Scenario 2: Warning profile
1. Raise gas or temperature moderately.
2. Hold until WARNING appears.
3. Show yellow indicator and ventilation response.

Scenario 3: Danger profile
1. Create sustained abnormal conditions safely.
2. Show ALARM_ACTIVE.
3. Show short press stop button mutes buzzer only.
4. Continue danger until CUTOFF_LOCKED.
5. Return to safe readings.
6. Show long press reset after safe window.

## 24. Final Checklist Before Submission

1. DHT22 is used, DHT11 is not used.
2. Multi-condition logic is implemented.
3. False alarm prevention techniques are active.
4. Gas cutoff mechanism is implemented and tested.
5. Stop alarm button mutes only, safety remains active.
6. State transitions are clearly shown on LCD and serial logs.
7. Power design includes margin and common ground.
8. Wiring is labeled and safe.
9. Three demo scenarios are practiced.

## 25. Important Safety Note

Do not run unsafe gas experiments in closed spaces.
Use controlled, supervised, low-risk simulation methods for academic testing.
