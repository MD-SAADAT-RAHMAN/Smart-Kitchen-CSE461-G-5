# CSE461: Introduction to Robotics - Project Proposal Draft

> **Section:** X | **Group:** X | **Semester:** Spring 2026
>
> **Note:** This draft is refined to the strongest 3 Arduino-only ideas based on the actual guideline, medium to medium-hard difficulty, and the availability of many similar builds/tutorials online.

---

## Member Details

| Name | ID |
|------|----|
|      |    |
|      |    |
|      |    |
|      |    |
|      |    |

**Submission Date:** _______________
**Instructors:** _______________

---

## Why These 3 Ideas

These 3 ideas were selected because they satisfy all guideline requirements and are realistic for a 5-person group where one person may do most of the technical work.

- All 3 use **Arduino Uno only**.
- All 3 are **autonomous and problem-solving**.
- All 3 use **at least 3 sensor types** and **at least 2 actuator types**.
- All 3 can be built as **stationary or semi-stationary systems**, so there is less mechanical risk than a moving robot.
- All 3 have **strong online support**: Arduino gas detection systems, Arduino parking gate systems, and Arduino people counter / room automation systems are common project categories with many tutorials, videos, and forum discussions.

---
---

# Project Idea 1 (First Preference)

## 1. Project Title

**Smart Kitchen Safety and Ventilation System**

## 2. Purpose

### Objective
Design and build an autonomous kitchen safety system that uses multi-condition logic to distinguish normal cooking from dangerous events, reduce false alarms, and automatically activate ventilation, alarms, and cutoff-level safety actions.

### Scope
The project is intended for household kitchens, dorm kitchens, or small food preparation areas. The system continuously monitors gas trend, temperature-humidity condition, and flame presence near the cooking area (with optional ambient-light context). Based on a multi-condition risk score and state-machine logic, it decides whether to open a vent, run an exhaust fan, trigger alarms, and execute cutoff-command output (LED in demo mode).

### Significance
- LPG gas leakage is a real and dangerous household problem in Bangladesh.
- Most low-cost systems only raise an alarm; this idea goes further by taking **automatic action**.
- It is highly relevant, socially useful, and likely to be appreciated by faculty.
- Similar systems are widely available online, so implementation help and troubleshooting resources are easy to find.

### Faculty Review Integration (Applied)
- **No simple yes/no threshold:** multi-condition risk scoring and state transitions are used.
- **False alarm prevention:** filtering, persistence timers, hysteresis, and cross-sensor validation are included.
- **Sensor update applied:** **DHT22** is used instead of DHT11.
- **Safety requirement applied:** automatic **cutoff command** is included (shown with LED in demo mode).
- **Usability requirement applied:** a mandatory **Stop Alarm button** is included.

## 3. Components

| Category | Component |
|----------|-----------|
| **Microcontroller** | Arduino Uno |
| **Sensor 1** | MQ-2 gas/smoke sensor |
| **Sensor 2** | DHT22 temperature and humidity sensor |
| **Sensor 3** | Flame sensor module (IR) |
| **Sensor 4 (Optional)** | LDR for kitchen activity / ambient-light context |
| **Actuator 1** | SG90 servo motor for ventilation flap |
| **Actuator 2** | Exhaust/DC fan controlled by relay |
| **Actuator 3** | Cutoff indicator LED (demo output; can drive solenoid valve in full-power version) |
| **Actuator 4** | I2C LCD 16x2 |
| **Additional Components** | Stop Alarm push button (mandatory), LEDs, buzzer, relay module(s), power distribution board (recommended), resistors, breadboard, jumper wires, enclosure/frame, power supply |

## 4. Cost Breakdown (Approx.)

| Component | Qty | Approx. Cost (BDT) |
|-----------|-----|--------------------|
| Arduino Uno | 1 | 700 |
| MQ-2 sensor | 1 | 200 |
| DHT22 sensor | 1 | 300 |
| Flame sensor module | 1 | 120 |
| LDR + resistor (optional) | 1 | 30 |
| SG90 servo | 1 | 200 |
| Exhaust/DC fan | 1 | 100 |
| Cutoff indicator LED + resistor | 1 | 20 |
| Relay module (1-channel for fan or 2-channel optional) | 1 | 120 |
| I2C LCD 16x2 | 1 | 350 |
| LEDs + buzzer | 1 set | 80 |
| Stop Alarm push button | 1 | 20 |
| Breadboard + jumper wires | 1 set | 200 |
| DIY enclosure/frame | 1 | 150 |
| Power supply set | 1 | 250 |
| Power distribution board (recommended) | 1 | 150 |
| **Total (Approx.)** |  | **~2,990 BDT** |

## 5. Functionality Breakdown

### Functionality 1: Multi-Condition Hazard Detection with False Alarm Prevention

**Overview:**
The system uses multiple sensors and state-based logic so normal cooking activity does not immediately trigger danger actions.

**Working Procedure:**
1. Arduino reads **MQ-2**, **DHT22**, and **flame sensor** values continuously (optional **LDR** context can be added).
2. Sensor data is stabilized using warm-up lockout, moving-average filtering, and outlier rejection.
3. A weighted risk score determines system states such as **NORMAL**, **COOKING_NORMAL**, **WARNING**, and **ALARM_ACTIVE**.
4. Persistence timers and hysteresis prevent one-sample spikes from creating false alarms.
5. Only sustained multi-condition danger transitions the system toward cutoff-level response.

### Functionality 2: Automatic Cutoff Command and Safety Actuation

**Overview:**
When confirmed danger persists, the system performs automatic protective actions including cutoff-command output.

**Working Procedure:**
1. If risk remains high for the configured confirmation window, the system enters **ALARM_ACTIVE** and then **CUTOFF_LOCKED**.
2. The **cutoff indicator LED** turns ON immediately to show cutoff command active (this output can drive a real valve in full-power deployment).
3. The **exhaust fan** is turned on and vent flap is opened using the **servo motor**.
4. Buzzer, LEDs, and LCD provide clear alarm and hazard status.
5. Cutoff command remains blocked until a continuous safe window is observed and manual reset conditions are met.

### Functionality 3: Stop Alarm Button and Safe Recovery Workflow

**Overview:**
The Stop Alarm button improves usability without allowing users to bypass safety protection.

**Working Procedure:**
1. A short press of the **Stop Alarm button** mutes the buzzer for a temporary window.
2. During mute, safety actions remain active: fan on, vent open, and cutoff command locked.
3. LCD explicitly shows safety-active status (for example, `ALARM MUTED - SAFETY ACTIVE`).
4. A long press is accepted only after safe readings persist for the configured recovery window.
5. On valid recovery, the system returns to normal state and allows controlled reset of alarm status.

---
---

# Project Idea 2 (Second Preference)

## 1. Project Title

**Smart Classroom Occupancy and Environment Controller**

## 2. Purpose

### Objective
Build an autonomous room monitoring and control system that counts people entering and leaving a room, monitors environmental conditions, and automatically controls ventilation and lighting to reduce energy waste.

### Scope
The project is designed for classrooms, offices, labs, or meeting rooms. It detects entry/exit movement, tracks occupancy count, and adjusts fan, vent, and lighting conditions according to room usage and environmental readings.

### Significance
- This solves a very visible real-world problem: fans and lights often remain on in empty rooms.
- It is directly relevant to academic buildings, so faculty can immediately relate to its usefulness.
- It demonstrates logic, automation, and multi-sensor decision-making rather than simple sensor reading.
- There are many Arduino tutorials for people counters, smart room automation, and occupancy-based energy systems.

## 3. Components

| Category | Component |
|----------|-----------|
| **Microcontroller** | Arduino Uno |
| **Sensor 1** | IR sensor pair for entry/exit counting |
| **Sensor 2** | DHT11 temperature and humidity sensor |
| **Sensor 3** | LDR for ambient light detection |
| **Actuator 1** | DC fan controlled by relay |
| **Actuator 2** | SG90 servo motor for vent/window flap |
| **Actuator 3** | I2C LCD 16x2 |
| **Additional Components** | White LEDs for room light simulation, buzzer, push button, relay module, breadboard, jumper wires, frame/enclosure, power supply |

## 4. Cost Breakdown (Approx.)

| Component | Qty | Approx. Cost (BDT) |
|-----------|-----|--------------------|
| Arduino Uno | 1 | 700 |
| IR sensors | 2 | 200 |
| DHT11 sensor | 1 | 150 |
| LDR + resistor | 1 | 30 |
| DC fan | 1 | 100 |
| SG90 servo | 1 | 200 |
| Relay module | 1 | 80 |
| I2C LCD 16x2 | 1 | 350 |
| LEDs + buzzer + button | - | 100 |
| Breadboard + jumper wires | - | 200 |
| DIY frame/enclosure | 1 | 200 |
| Power supply | 1 | 200 |
| **Total (Approx.)** |  | **~2,510 BDT** |

## 5. Functionality Breakdown

### Functionality 1: Bidirectional Occupancy Counting

**Overview:**
Two IR sensors identify whether a person is entering or leaving the room based on trigger order. The occupancy count is updated automatically.

**Working Procedure:**
1. Two **IR sensors** are placed at the doorway with a small gap between them.
2. If the outer sensor triggers before the inner sensor, Arduino counts it as **entry**.
3. If the inner sensor triggers before the outer sensor, Arduino counts it as **exit**.
4. The LCD shows the live room count and maximum capacity.
5. If occupancy reaches a limit, the buzzer sounds briefly and the LCD displays a full-room warning.

### Functionality 2: Temperature-Controlled Ventilation

**Overview:**
When the room is occupied, the fan and vent are controlled according to temperature and humidity readings.

**Working Procedure:**
1. The **DHT11** continuously measures temperature and humidity.
2. If the room is empty, the system keeps the fan off and closes the vent to save power.
3. If the room is occupied and temperature rises above the comfort threshold, the relay turns the fan on.
4. The **servo motor** opens the vent partially or fully depending on temperature level.
5. The LCD displays messages such as `ROOM EMPTY`, `FAN ON`, or `HIGH TEMP`.

### Functionality 3: Occupancy-Based Smart Lighting

**Overview:**
The LDR measures ambient light, and the system turns on LED room lights only when the room is occupied and natural light is insufficient.

**Working Procedure:**
1. The **LDR** provides an analog light-level reading.
2. If the room is empty, all room lights remain off regardless of light level.
3. If the room is occupied and the light level is low, Arduino turns on white LEDs.
4. If the room is occupied and the light level is medium, only part of the LEDs turn on.
5. The LCD alternates between occupancy status, environmental readings, and lighting status.

---
---

# Project Idea 3 (Third Preference)

## 1. Project Title

**Smart Automated Parking Gate System with Vehicle Counting**

## 2. Purpose

### Objective
Design and build an autonomous parking gate system that detects vehicles, opens and closes a gate automatically, counts available parking spaces, and denies entry when the lot is full.

### Scope
This project is intended for small apartment, office, or campus parking areas. It simulates an automated gate controller using vehicle detection at entry and exit points, a servo-controlled barrier, and a live parking count display.

### Significance
- Parking management is a practical daily problem in urban areas.
- The project is easy to demonstrate physically, which is useful during evaluation.
- It has enough logic to be accepted as a proper robotics project without becoming too risky.
- There are many online examples for Arduino parking barriers, servo gates, IR counters, and ultrasonic detection.

## 3. Components

| Category | Component |
|----------|-----------|
| **Microcontroller** | Arduino Uno |
| **Sensor 1** | HC-SR04 ultrasonic sensor for entry detection |
| **Sensor 2** | IR sensor for exit detection |
| **Sensor 3** | LDR for day/night mode |
| **Actuator 1** | SG90 servo motor for barrier gate |
| **Actuator 2** | I2C LCD 16x2 |
| **Actuator 3** | LEDs and buzzer for traffic/status indication |
| **Additional Components** | Push button, breadboard, jumper wires, resistors, small parking model/frame, power supply |

## 4. Cost Breakdown (Approx.)

| Component | Qty | Approx. Cost (BDT) |
|-----------|-----|--------------------|
| Arduino Uno | 1 | 700 |
| HC-SR04 sensor | 1 | 150 |
| IR sensor | 1 | 100 |
| LDR + resistor | 1 | 30 |
| SG90 servo | 1 | 200 |
| I2C LCD 16x2 | 1 | 350 |
| LEDs + buzzer | - | 80 |
| Push button | 1 | 20 |
| Breadboard + jumper wires | - | 200 |
| DIY parking frame | 1 | 200 |
| Power supply | 1 | 200 |
| **Total (Approx.)** |  | **~2,230 BDT** |

## 5. Functionality Breakdown

### Functionality 1: Vehicle Detection and Gate Opening

**Overview:**
The system detects a vehicle at the entry point and opens the gate automatically if parking space is available.

**Working Procedure:**
1. The **ultrasonic sensor** detects an approaching vehicle at the entry lane.
2. Arduino checks the current parking count.
3. If the parking lot is not full, the **servo motor** raises the barrier gate.
4. After the vehicle passes, the gate closes automatically.
5. The LCD shows `GATE OPEN`, `GATE CLOSED`, or `LOT FULL`.

### Functionality 2: Exit Detection and Space Count Update

**Overview:**
An IR sensor at the exit point detects vehicles leaving and updates the available parking count.

**Working Procedure:**
1. The **IR sensor** detects a vehicle passing through the exit side.
2. Arduino decreases the occupied-space count when a vehicle exits.
3. The LCD continuously displays available spots.
4. If the lot was previously full, the system returns to entry-allowed mode.
5. A push button can be used for manual count reset during testing or maintenance.

### Functionality 3: Day/Night Status Lighting and Alert Indication

**Overview:**
The LDR detects ambient light and changes the gate-area lighting and status display behavior between day and night mode.

**Working Procedure:**
1. The **LDR** reads ambient brightness.
2. In daylight mode, only standard status LEDs are active.
3. In night mode, extra white LEDs turn on to illuminate the gate area.
4. When the lot is full, the red LED and buzzer alert the driver.
5. The LCD alternates between parking count, gate state, and day/night status.

---
---

# Project Idea 4 (Additional Strong Option)

## 1. Project Title

**Smart Water Tank Monitoring and Pump Protection System**

## 2. Purpose

### Objective
Design and build an autonomous water tank management system that monitors tank level, detects overflow and leakage conditions, and automatically controls water pumping to reduce water waste and protect the pump from dry running.

### Scope
This project is intended for home rooftops, apartment buildings, and hostels where overhead water tanks are commonly used. The system measures the water level inside the tank, checks for overflow or leakage, and turns the pump on or off automatically while displaying system status and alarms.

### Significance
- Water overflow and dry-run pump damage are common real-world problems in Bangladesh.
- The project has strong practical value and is immediately understandable during evaluation.
- There are many Arduino water tank controller tutorials, so components, wiring methods, and sample logic are easy to find online.
- This idea can be designed with lower single-point dependency by using more than one sensing method for water-related decisions.

## 3. Components

| Category | Component |
|----------|-----------|
| **Microcontroller** | Arduino Uno |
| **Sensor 1** | HC-SR04 ultrasonic sensor for tank level |
| **Sensor 2** | Water leak sensor near overflow or floor area |
| **Sensor 3** | Float switch / backup level switch |
| **Actuator 1** | Relay-controlled pump |
| **Actuator 2** | I2C LCD 16x2 |
| **Actuator 3** | Buzzer and status LEDs |
| **Additional Components** | Relay module, push button, breadboard, jumper wires, tank model/frame, power supply |

## 4. Cost Breakdown (Approx.)

| Component | Qty | Approx. Cost (BDT) |
|-----------|-----|--------------------|
| Arduino Uno | 1 | 700 |
| HC-SR04 sensor | 1 | 150 |
| Water leak sensor | 1 | 120 |
| Float switch / level switch | 1 | 150 |
| Relay module | 1 | 80 |
| I2C LCD 16x2 | 1 | 350 |
| LEDs + buzzer | - | 80 |
| Push button | 1 | 20 |
| Breadboard + jumper wires | - | 200 |
| DIY tank model / frame | 1 | 200 |
| Power supply | 1 | 200 |
| **Total (Approx.)** |  | **~2,250 BDT** |

## 5. Functionality Breakdown

### Functionality 1: Automatic Tank Level Monitoring and Pump Control

**Overview:**
The system measures water level continuously and turns the pump on when the tank is low and off when the tank is full.

**Working Procedure:**
1. The **ultrasonic sensor** is mounted at the top of the tank to measure distance to the water surface.
2. Arduino converts the measured distance into approximate tank-fill percentage.
3. If the water level goes below the low threshold, Arduino activates the **relay**, which turns the pump on.
4. If the level reaches the full threshold, Arduino turns the pump off automatically.
5. The LCD shows the current water level, pump state, and warning status.

### Functionality 2: Overflow and Leakage Detection with Cross-Check

**Overview:**
The system uses a second water-related sensor so the project is not fully dependent on one sensor reading.

**Working Procedure:**
1. A **water leak sensor** is placed near the overflow outlet or around the floor area.
2. A **float switch** acts as a backup full-level or low-level confirmation sensor.
3. If the ultrasonic sensor indicates near-full level and the float switch confirms full state, the pump is turned off immediately.
4. If leakage or overflow water is detected unexpectedly, the buzzer turns on and the LCD shows `LEAK DETECTED` or `OVERFLOW ALERT`.
5. This dual-check approach reduces single-point dependency and improves reliability.

### Functionality 3: Safety Alerts and Manual Recovery Mode

**Overview:**
The system provides warnings during abnormal conditions and supports safe manual recovery.

**Working Procedure:**
1. Green LED indicates normal operation, yellow LED indicates warning, and red LED indicates fault.
2. If the pump runs too long without expected level rise, the system assumes possible dry-run or supply failure and shuts the pump off.
3. The buzzer gives different alert patterns for overflow, leakage, and pump timeout.
4. A push button allows manual reset after inspection.
5. The LCD alternates between tank level, pump state, and fault messages.

---
---

# Project Idea 5 (Additional Strong Option)

## 1. Project Title

**Smart Medicine Reminder and Safe Dispenser System**

## 2. Purpose

### Objective
Build an autonomous medicine reminder system that alerts users at scheduled times, opens a medicine compartment automatically, and checks whether the medicine has been collected, helping elderly or busy users maintain regular medication habits.

### Scope
This project is designed for home use, especially for elderly people or patients taking regular medicine. The system reminds the user, opens a medicine compartment, detects user presence and medicine pickup, and provides visual and audio confirmation or missed-dose alerts.

### Significance
- Medicine non-adherence is a major healthcare problem and has strong social relevance.
- The project stands out from common student project ideas while still being buildable with standard Arduino components.
- There are many Arduino pill reminder, dispenser, servo-lid, and alert system examples online.
- It can be designed with multiple confirmation steps so it is not dependent on one uncertain sensor reading.

## 3. Components

| Category | Component |
|----------|-----------|
| **Microcontroller** | Arduino Uno |
| **Sensor 1** | IR sensor for medicine pickup detection |
| **Sensor 2** | HC-SR04 ultrasonic sensor for user presence |
| **Sensor 3** | LDR for day/night mode |
| **Actuator 1** | SG90 servo motor for compartment lid |
| **Actuator 2** | I2C LCD 16x2 |
| **Actuator 3** | Buzzer and status LEDs |
| **Additional Components** | Push button for acknowledge/snooze, breadboard, jumper wires, medicine box/frame, resistors, power supply |

## 4. Cost Breakdown (Approx.)

| Component | Qty | Approx. Cost (BDT) |
|-----------|-----|--------------------|
| Arduino Uno | 1 | 700 |
| IR sensor | 1 | 100 |
| HC-SR04 sensor | 1 | 150 |
| LDR + resistor | 1 | 30 |
| SG90 servo | 1 | 200 |
| I2C LCD 16x2 | 1 | 350 |
| LEDs + buzzer | - | 80 |
| Push button | 1 | 20 |
| Breadboard + jumper wires | - | 200 |
| Medicine box / frame | 1 | 150 |
| Power supply | 1 | 200 |
| **Total (Approx.)** |  | **~2,180 BDT** |

## 5. Functionality Breakdown

### Functionality 1: Timed Reminder and Automatic Compartment Opening

**Overview:**
At preset intervals, the system alerts the user and opens the medicine compartment automatically.

**Working Procedure:**
1. Arduino uses time tracking with `millis()` for reminder intervals during the demo.
2. When reminder time arrives, the buzzer sounds and the yellow LED turns on.
3. The LCD displays `MEDICINE TIME` and the current dose number.
4. The **servo motor** opens the medicine lid.
5. If the dose is not collected within the allowed time, the system marks it as missed and gives a stronger warning.

### Functionality 2: Medicine Pickup Confirmation with Dual Check

**Overview:**
The system uses two sensors so that medicine pickup is not confirmed by only one uncertain reading.

**Working Procedure:**
1. The **ultrasonic sensor** checks whether a person is near the device.
2. The **IR sensor** checks whether a hand reaches into the medicine compartment.
3. Only when both conditions occur in sequence does Arduino confirm `MEDICINE TAKEN`.
4. The LCD shows a success message and the green LED turns on.
5. If the user is near but the compartment is not accessed, the reminder continues.

### Functionality 3: Day/Night Mode and Missed-Dose Alert Handling

**Overview:**
The system changes reminder behavior based on surrounding light and keeps a simple status summary.

**Working Procedure:**
1. The **LDR** identifies day and night conditions.
2. In daytime mode, reminders use full display brightness and stronger buzzer signals.
3. In nighttime mode, the LCD and buzzer use softer behavior to avoid unnecessary disturbance.
4. If a dose is missed, the red LED blinks and the LCD displays `DOSE MISSED`.
5. A push button can acknowledge or snooze the reminder temporarily.

---
---

# Summary Comparison Table

| Criterion | Idea 1: Kitchen Safety | Idea 2: Room Controller | Idea 3: Parking Gate | Idea 4: Water Tank System | Idea 5: Medicine Reminder |
|-----------|------------------------|-------------------------|----------------------|----------------------------|----------------------------|
| **Difficulty** | Medium to Medium-Hard | Medium to Medium-Hard | Medium | Medium | Medium |
| **Approval Chance** | Very high | High | High | Very high | High |
| **Online Resources** | Very high | High | Very high | Very high | High |
| **Mechanical Complexity** | Low | Low | Medium | Low | Low |
| **Coding Complexity** | Medium | Medium-Hard | Medium | Medium | Medium |
| **Demo Impact** | Strong | Strong | Strong | Strong | Medium-Strong |
| **Risk Level** | Medium | Medium | Low-Medium | Low-Medium | Medium |
| **Best Use** | Social impact + safety | Smart campus / energy saving | Clean practical demo | Practical household automation | Social impact + healthcare |

---

# Recommended Submission Order

1. **Smart Kitchen Safety and Ventilation System**
Why: strongest real-world problem, strong faculty appeal, good technical depth, and lots of build references online.

2. **Smart Classroom Occupancy and Environment Controller**
Why: practical and relevant to academic settings, more logic-based, good if you want a slightly stronger technical impression.

3. **Smart Automated Parking Gate System with Vehicle Counting**
Why: easiest of the three to physically demonstrate and lowest risk if time becomes short.

4. **Smart Water Tank Monitoring and Pump Protection System**
Why: very practical, low mechanical risk, strong online resource availability, and easier to justify as a reliability-focused project.

5. **Smart Medicine Reminder and Safe Dispenser System**
Why: socially meaningful and distinctive, with manageable hardware and clear faculty appeal if you want a more unique proposal.

---

# Work Distribution for a 5-Person Group

- **You:** Arduino code, system integration, testing logic.
- **Member 2:** Physical frame / enclosure build.
- **Member 3:** Wiring and component arrangement.
- **Member 4:** Cost table, report writing, citations.
- **Member 5:** Presentation slides, demo flow, video/photos.

This split keeps the technical core manageable if most implementation is done by you.

---

# Next Step

Copy these 3 ideas into the official proposal template and fill in your group details. If you want, I can next turn this into a more polished faculty-ready version with slightly more formal wording for direct submission.
