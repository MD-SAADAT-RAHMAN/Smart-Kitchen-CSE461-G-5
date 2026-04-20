# Smart Kitchen Safety and Ventilation System

## Project Decision
Your faculty selected Project 1.

This guide converts the faculty comments into a buildable plan for your final project.

## Faculty Requirements Mapped
- Selected idea: Smart Kitchen Safety and Ventilation System
- Relevance: High (kitchen safety is a major local issue)
- Avoid simple yes/no detection: Use multi-condition logic and state-based decisions
- Reduce false alarms: Use filtering, time confirmation, cross-sensor validation, and hysteresis
- Sensor update: Use DHT22 (not DHT11)
- Safety action required: Include cutoff system
- Usability requirement: Add stop alarm button

## Final System Objective
Build an autonomous kitchen safety controller that:
- Detects dangerous gas-leak and fire-risk conditions
- Distinguishes normal cooking from dangerous events
- Performs automatic safety actions (ventilation + gas cutoff)
- Allows user alarm acknowledgement without disabling safety controls

## Recommended Hardware (Updated)
### Core Controller
- Arduino Uno

### Sensors
- MQ-2 gas/smoke sensor (gas trend + smoke indicator)
- DHT22 temperature/humidity sensor (higher accuracy and reliability)
- Flame sensor module (IR flame detection)
- Optional but useful: LDR or ambient light sensor for activity context

### Actuators and Safety
- Exhaust fan (relay controlled)
- Servo motor for vent flap
- Buzzer (alarm)
- Red/Yellow/Green LEDs (status)
- LCD 16x2 I2C (status and messages)
- Solenoid gas valve + relay or MOSFET driver (cutoff system)

### Buttons
- Stop Alarm button (mandatory)
- Optional Reset/Service button

## Multi-Condition Logic (Key Improvement)
Do not trigger alarm from one sensor only.
Use a risk scoring and state machine approach.

### A. Signals to Monitor
- Gas level relative to calibrated baseline (MQ-2)
- Rate of gas increase (delta over time)
- Temperature level and temperature rise rate (DHT22)
- Flame presence and duration
- Optional context: ambient light / expected cooking period

### B. Proposed System States
- NORMAL
- COOKING_NORMAL
- WARNING
- ALARM_ACTIVE
- CUTOFF_LOCKED

### C. Example Decision Rules
Use a weighted score and persistence timer:
- Gas high only for 1 to 2 seconds: no alarm yet
- Gas high plus rising temperature for 10 seconds: WARNING
- Gas high plus rapid rise plus flame anomaly or no-cooking-context: ALARM_ACTIVE
- ALARM_ACTIVE for more than X seconds: CUTOFF_LOCKED

### D. Suggested Risk Score Model
Use a 0 to 10 score:
- Gas level abnormal: +3
- Gas rising fast: +2
- Temperature above warning threshold: +2
- Temperature rising quickly: +1
- Flame detected unexpectedly: +2
- Optional context says no active cooking: +1

State thresholds:
- 0 to 3: NORMAL or COOKING_NORMAL
- 4 to 6: WARNING
- 7 to 10: ALARM_ACTIVE
- If ALARM_ACTIVE persists beyond cutoff timer: CUTOFF_LOCKED

## False Alarm Prevention Strategy
Implement all of these:
- Sensor warm-up lockout for MQ-2 at startup
- Moving average filter for gas and temperature
- Time confirmation (condition must hold for N seconds)
- Hysteresis (different enter and exit thresholds)
- Sensor cross-check (never decide on one reading only)
- Outlier rejection for sudden single-sample spikes
- Cooldown logic before returning to NORMAL

## Cutoff System Design
When danger is confirmed:
- Close gas solenoid valve
- Turn ON exhaust fan
- Open vent flap fully
- Keep alarm active until acknowledgement and safe conditions

Cutoff release policy:
- Do not reopen gas immediately after alarm stop
- Require safe readings for a continuous safety window (for example 60 seconds)
- Require manual reset (long press or dedicated reset button)

## Stop Alarm Button Behavior (Mandatory)
Define button behavior clearly:
- Short press in ALARM_ACTIVE:
  - Silence buzzer for a temporary mute window (for example 120 seconds)
  - Keep fan ON, vent OPEN, and cutoff active
  - Show LCD message: ALARM MUTED - SAFETY ACTIVE
- Long press (for example 3 seconds) only after safe window:
  - Clear alarm
  - Return to NORMAL
  - Allow controlled gas restore if design permits

Important rule:
- Stop Alarm button should mute sound, not disable safety logic.

## Practical Wiring Plan
- MQ-2 to analog input
- DHT22 to digital pin with required pull-up resistor
- Flame sensor to digital input (and optional analog if module supports)
- Fan relay, valve relay, buzzer, LEDs to digital outputs
- Stop button to digital input with pull-down or INPUT_PULLUP logic
- LCD via I2C SDA/SCL

## Firmware Module Plan
Implement code in modules:
- sensor_reading module
- filtering_and_baseline module
- risk_engine module
- state_machine module
- actuator_control module
- ui_and_button module
- data_logging_serial module

## Step-by-Step Build Plan
### Step 1: Requirements Freeze
- Confirm all thresholds, timers, and state names with faculty
- Finalize component list with DHT22 and gas cutoff valve

### Step 2: Hardware Assembly
- Build breadboard prototype
- Verify each sensor and actuator independently
- Confirm relay and valve switching safety

### Step 3: Sensor Calibration
- Record baseline gas in clean-air environment
- Log kitchen-like normal cooking readings
- Log abnormal test scenarios carefully and safely

### Step 4: Implement State Machine
- Add NORMAL and COOKING_NORMAL first
- Add WARNING and ALARM_ACTIVE
- Add CUTOFF_LOCKED last

### Step 5: Add Button Logic
- Implement short press mute behavior
- Implement long press reset behavior
- Validate that safety actions remain active during mute

### Step 6: False Alarm Tuning
- Tune filters, persistence timers, and hysteresis
- Re-test with repeated normal cooking sessions

### Step 7: Demo Preparation
- Create 3 scripted demos:
  - Normal cooking (no false alarm)
  - Warning condition
  - Confirmed danger with cutoff + stop button behavior

## Testing Checklist
- Normal cooking does not trigger ALARM_ACTIVE
- Brief gas spike does not trigger cutoff
- Sustained dangerous condition triggers cutoff reliably
- Stop button mutes buzzer but keeps safety actions active
- Safe recovery requires safe window and reset
- DHT22 values remain stable and believable

## Suggested Project Timeline (4 Weeks)
- Week 1: Components, wiring, independent sensor tests
- Week 2: Core firmware, filtering, state machine
- Week 3: False alarm reduction, cutoff and button integration
- Week 4: Final tuning, report, slides, and demo rehearsal

## What You Should Do Next (Immediate)
1. Buy DHT22, flame sensor, and gas solenoid valve parts first.
2. Build a minimal prototype: MQ-2 + DHT22 + buzzer + stop button.
3. Implement and test state machine without cutoff first.
4. Add cutoff relay logic only after stable detection.
5. Collect 2 to 3 days of test logs and tune thresholds.
6. Prepare demo script showing no false alarms during normal cooking.

## Report Writing Tips
In your report, emphasize:
- Why single-threshold systems fail in real kitchens
- Your multi-condition logic design
- Quantitative false-alarm reduction after tuning
- Safe behavior of cutoff and alarm mute
- Why DHT22 improved reliability over DHT11

## Safety Note
Gas-related testing must be controlled and supervised.
Use low-risk simulation when possible, and avoid unsafe open-gas experiments in closed rooms.