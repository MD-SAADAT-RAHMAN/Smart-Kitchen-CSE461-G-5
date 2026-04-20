#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// =========================
// Hardware pin map
// =========================
const uint8_t PIN_MQ2 = A0;
const uint8_t PIN_STOP_BUTTON = 2;
const uint8_t PIN_FLAME = 3;
const uint8_t PIN_BUZZER = 4;
const uint8_t PIN_RELAY_FAN = 5;
const uint8_t PIN_RELAY_VALVE = 6;
const uint8_t PIN_LED_YELLOW = 7;
const uint8_t PIN_LED_RED = 8;
const uint8_t PIN_LED_GREEN = 9;
const uint8_t PIN_SERVO = 10;
const uint8_t PIN_RESET_BUTTON = 11;  // Optional
const uint8_t PIN_DHT = 12;

// =========================
// Build-time configuration
// =========================
const bool USE_RESET_BUTTON = false;
const bool RELAY_ACTIVE_LOW = true;
const bool FLAME_ACTIVE_LOW = true;

// Valve behavior depends on your actual valve type.
// true  = relay ON means gas path is ALLOWED.
// false = relay ON means gas path is CUTOFF.
const bool VALVE_ENERGIZE_TO_ALLOW_GAS = true;

const uint8_t LCD_I2C_ADDR = 0x27;

// Servo angles (tune for your mechanics)
const int SERVO_ANGLE_CLOSED = 20;
const int SERVO_ANGLE_MID = 90;
const int SERVO_ANGLE_OPEN = 160;

// Timing
const unsigned long MQ2_WARMUP_MS = 120000UL;
const unsigned long SENSOR_SAMPLE_INTERVAL_MS = 250UL;
const unsigned long DHT_SAMPLE_INTERVAL_MS = 1000UL;
const unsigned long LCD_UPDATE_INTERVAL_MS = 500UL;
const unsigned long SERIAL_UPDATE_INTERVAL_MS = 1000UL;
const unsigned long WARNING_PERSIST_MS = 8000UL;
const unsigned long ALARM_PERSIST_MS = 12000UL;
const unsigned long CUTOFF_AFTER_ALARM_MS = 10000UL;
const unsigned long SAFE_WINDOW_MS = 60000UL;
const unsigned long BUZZER_MUTE_MS = 120000UL;
const unsigned long BUTTON_DEBOUNCE_MS = 35UL;
const unsigned long LONG_PRESS_MS = 3000UL;
const unsigned long RED_FLASH_MS = 400UL;
const unsigned long GREEN_BLINK_MS = 600UL;

// Thresholds (starter values; tune from logs)
const float TEMP_WARNING_C = 38.0f;
const float TEMP_ALARM_C = 45.0f;
const float TEMP_RISE_WARNING_C_PER_MIN = 1.2f;
const float TEMP_RISE_ALARM_C_PER_MIN = 2.5f;
const int GAS_WARN_DELTA_ENTER = 120;
const int GAS_WARN_DELTA_EXIT = 90;
const int GAS_ALARM_DELTA_ENTER = 220;
const int GAS_ALARM_DELTA_EXIT = 180;
const float GAS_RISE_FAST_COUNTS_PER_SEC = 6.0f;
const unsigned long FLAME_PERSIST_MS = 1500UL;

const uint8_t FILTER_WINDOW = 10;

DHT dht(PIN_DHT, DHT22);
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);
Servo ventServo;

enum SystemState {
  STATE_WARMUP,
  STATE_NORMAL,
  STATE_COOKING_NORMAL,
  STATE_WARNING,
  STATE_ALARM_ACTIVE,
  STATE_CUTOFF_LOCKED
};

struct ButtonTracker {
  bool lastRawPressed;
  bool stablePressed;
  bool longHandled;
  unsigned long lastRawChangeMs;
  unsigned long pressStartMs;
};

enum ButtonEvent {
  BUTTON_NONE,
  BUTTON_SHORT_PRESS,
  BUTTON_LONG_PRESS
};

SystemState currentState = STATE_WARMUP;

int gasBuffer[FILTER_WINDOW] = {0};
float tempBuffer[FILTER_WINDOW] = {0};
uint8_t gasIndex = 0;
uint8_t tempIndex = 0;
uint8_t gasCount = 0;
uint8_t tempCount = 0;
long gasSum = 0;
float tempSum = 0.0f;

int gasRaw = 0;
int gasAvg = 0;
float tempC = 25.0f;
float tempAvg = 25.0f;
float tempRiseCPerMin = 0.0f;
float gasRiseCountsPerSec = 0.0f;

int gasBaseline = 0;
bool baselineLocked = false;

bool flameDetected = false;
bool flamePersistent = false;
unsigned long flameStartMs = 0;

bool alarmLatched = false;
bool cutoffLocked = false;
unsigned long warningCandidateStartMs = 0;
unsigned long alarmCandidateStartMs = 0;
unsigned long cutoffCandidateStartMs = 0;
unsigned long safeSinceMs = 0;
unsigned long buzzerMuteUntilMs = 0;

unsigned long startupMs = 0;
unsigned long lastSensorMs = 0;
unsigned long lastDhtMs = 0;
unsigned long lastLcdMs = 0;
unsigned long lastSerialMs = 0;
unsigned long lastGasRateMs = 0;
unsigned long lastTempRateMs = 0;
unsigned long lastRedFlashMs = 0;
unsigned long lastGreenBlinkMs = 0;

int lastGasForRate = 0;
float lastTempForRate = 25.0f;

bool redLedFlashState = false;
bool greenLedBlinkState = false;

ButtonTracker stopButton = {false, false, false, 0, 0};
ButtonTracker resetButton = {false, false, false, 0, 0};

void writeRelay(uint8_t pin, bool on) {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(pin, on ? LOW : HIGH);
  } else {
    digitalWrite(pin, on ? HIGH : LOW);
  }
}

bool readFlameRawDetected() {
  bool pinState = digitalRead(PIN_FLAME);
  return FLAME_ACTIVE_LOW ? !pinState : pinState;
}

ButtonEvent updateButton(ButtonTracker &btn, uint8_t pin, unsigned long nowMs) {
  bool rawPressed = (digitalRead(pin) == LOW);  // INPUT_PULLUP

  if (rawPressed != btn.lastRawPressed) {
    btn.lastRawPressed = rawPressed;
    btn.lastRawChangeMs = nowMs;
  }

  if ((nowMs - btn.lastRawChangeMs) >= BUTTON_DEBOUNCE_MS && rawPressed != btn.stablePressed) {
    btn.stablePressed = rawPressed;

    if (btn.stablePressed) {
      btn.pressStartMs = nowMs;
      btn.longHandled = false;
    } else {
      if (!btn.longHandled) {
        return BUTTON_SHORT_PRESS;
      }
    }
  }

  if (btn.stablePressed && !btn.longHandled && (nowMs - btn.pressStartMs) >= LONG_PRESS_MS) {
    btn.longHandled = true;
    return BUTTON_LONG_PRESS;
  }

  return BUTTON_NONE;
}

void pushGasSample(int value) {
  if (gasCount < FILTER_WINDOW) {
    gasBuffer[gasIndex] = value;
    gasSum += value;
    gasCount++;
  } else {
    gasSum -= gasBuffer[gasIndex];
    gasBuffer[gasIndex] = value;
    gasSum += value;
  }

  gasIndex = (gasIndex + 1) % FILTER_WINDOW;
  gasAvg = gasSum / max((uint8_t)1, gasCount);
}

void pushTempSample(float value) {
  if (tempCount < FILTER_WINDOW) {
    tempBuffer[tempIndex] = value;
    tempSum += value;
    tempCount++;
  } else {
    tempSum -= tempBuffer[tempIndex];
    tempBuffer[tempIndex] = value;
    tempSum += value;
  }

  tempIndex = (tempIndex + 1) % FILTER_WINDOW;
  tempAvg = tempSum / max((uint8_t)1, tempCount);
}

void updateSensorReadings(unsigned long nowMs) {
  if ((nowMs - lastSensorMs) >= SENSOR_SAMPLE_INTERVAL_MS) {
    lastSensorMs = nowMs;

    gasRaw = analogRead(PIN_MQ2);

    // Very basic outlier suppression for single-sample spikes.
    if (gasCount > 0) {
      int delta = abs(gasRaw - gasAvg);
      if (delta > 300) {
        gasRaw = gasAvg;
      }
    }

    pushGasSample(gasRaw);

    flameDetected = readFlameRawDetected();
    if (flameDetected) {
      if (flameStartMs == 0) {
        flameStartMs = nowMs;
      }
      flamePersistent = (nowMs - flameStartMs) >= FLAME_PERSIST_MS;
    } else {
      flameStartMs = 0;
      flamePersistent = false;
    }
  }

  if ((nowMs - lastDhtMs) >= DHT_SAMPLE_INTERVAL_MS) {
    lastDhtMs = nowMs;

    float newTemp = dht.readTemperature();
    if (!isnan(newTemp)) {
      tempC = newTemp;
      pushTempSample(tempC);

      if (lastTempRateMs > 0) {
        unsigned long dt = nowMs - lastTempRateMs;
        if (dt > 0) {
          tempRiseCPerMin = ((tempAvg - lastTempForRate) * 60000.0f) / (float)dt;
        }
      }
      lastTempForRate = tempAvg;
      lastTempRateMs = nowMs;
    }
  }

  if ((nowMs - lastGasRateMs) >= DHT_SAMPLE_INTERVAL_MS) {
    if (lastGasRateMs > 0) {
      unsigned long dt = nowMs - lastGasRateMs;
      if (dt > 0) {
        gasRiseCountsPerSec = ((float)(gasAvg - lastGasForRate) * 1000.0f) / (float)dt;
      }
    }
    lastGasForRate = gasAvg;
    lastGasRateMs = nowMs;
  }
}

bool isWarmupActive(unsigned long nowMs) {
  return (nowMs - startupMs) < MQ2_WARMUP_MS;
}

void updateGasBaseline(unsigned long nowMs) {
  if (isWarmupActive(nowMs)) {
    gasBaseline = gasAvg;
    return;
  }

  if (!baselineLocked) {
    gasBaseline = gasAvg;
    baselineLocked = true;
    return;
  }

  // Slow baseline tracking only in non-danger states.
  if (!alarmLatched && (currentState == STATE_NORMAL || currentState == STATE_COOKING_NORMAL)) {
    if (abs(gasAvg - gasBaseline) < 80) {
      gasBaseline = (int)(0.995f * gasBaseline + 0.005f * gasAvg);
    }
  }
}

int computeRiskScore() {
  int score = 0;

  bool gasAbnormal = gasAvg >= (gasBaseline + GAS_WARN_DELTA_ENTER);
  bool gasRisingFast = gasRiseCountsPerSec >= GAS_RISE_FAST_COUNTS_PER_SEC;
  bool tempWarn = tempAvg >= TEMP_WARNING_C;
  bool tempRising = tempRiseCPerMin >= TEMP_RISE_WARNING_C_PER_MIN;
  bool flameUnexpected = flamePersistent;

  if (gasAbnormal) {
    score += 3;
  }
  if (gasRisingFast) {
    score += 2;
  }
  if (tempWarn) {
    score += 2;
  }
  if (tempRising) {
    score += 1;
  }
  if (flameUnexpected) {
    score += 2;
  }

  if (score > 10) {
    score = 10;
  }
  return score;
}

bool isCookingContext() {
  bool gasSlight = gasAvg >= (gasBaseline + 60) && gasAvg < (gasBaseline + GAS_WARN_DELTA_ENTER);
  bool tempSlight = tempAvg >= (TEMP_WARNING_C - 3.0f) && tempAvg < TEMP_WARNING_C;
  return gasSlight || tempSlight;
}

bool isDangerStillHigh(int riskScore) {
  bool gasAlarm = gasAvg >= (gasBaseline + GAS_ALARM_DELTA_ENTER);
  bool tempAlarm = tempAvg >= TEMP_ALARM_C;
  bool tempRiseAlarm = tempRiseCPerMin >= TEMP_RISE_ALARM_C_PER_MIN;
  return riskScore >= 7 || gasAlarm || tempAlarm || tempRiseAlarm || flamePersistent;
}

bool isSafeCondition() {
  bool gasSafe = gasAvg <= (gasBaseline + GAS_WARN_DELTA_EXIT);
  bool tempSafe = tempAvg < (TEMP_WARNING_C - 1.5f);
  bool noFlame = !flamePersistent;
  return gasSafe && tempSafe && noFlame;
}

void resetAlarmLatches() {
  alarmLatched = false;
  cutoffLocked = false;
  warningCandidateStartMs = 0;
  alarmCandidateStartMs = 0;
  cutoffCandidateStartMs = 0;
  safeSinceMs = 0;
  buzzerMuteUntilMs = 0;
}

void updateStateMachine(unsigned long nowMs) {
  if (isWarmupActive(nowMs)) {
    currentState = STATE_WARMUP;
    return;
  }

  int riskScore = computeRiskScore();

  if (!alarmLatched) {
    bool warningCondition = (riskScore >= 4);
    bool alarmCondition = (riskScore >= 7);

    if (alarmCondition) {
      if (alarmCandidateStartMs == 0) {
        alarmCandidateStartMs = nowMs;
      }
      if ((nowMs - alarmCandidateStartMs) >= ALARM_PERSIST_MS) {
        alarmLatched = true;
        currentState = STATE_ALARM_ACTIVE;
        cutoffCandidateStartMs = nowMs;
        warningCandidateStartMs = 0;
      }
    } else {
      alarmCandidateStartMs = 0;
    }

    if (!alarmLatched) {
      if (warningCondition) {
        if (warningCandidateStartMs == 0) {
          warningCandidateStartMs = nowMs;
        }
        if ((nowMs - warningCandidateStartMs) >= WARNING_PERSIST_MS) {
          currentState = STATE_WARNING;
        }
      } else {
        warningCandidateStartMs = 0;
        currentState = isCookingContext() ? STATE_COOKING_NORMAL : STATE_NORMAL;
      }
    }
  } else {
    bool dangerHigh = isDangerStillHigh(riskScore);

    if (dangerHigh) {
      safeSinceMs = 0;
      if (!cutoffLocked) {
        if (cutoffCandidateStartMs == 0) {
          cutoffCandidateStartMs = nowMs;
        }
        if ((nowMs - cutoffCandidateStartMs) >= CUTOFF_AFTER_ALARM_MS) {
          cutoffLocked = true;
        }
      }
    } else {
      cutoffCandidateStartMs = 0;
      if (safeSinceMs == 0) {
        safeSinceMs = nowMs;
      }
    }

    currentState = cutoffLocked ? STATE_CUTOFF_LOCKED : STATE_ALARM_ACTIVE;
  }
}

void handleButtons(unsigned long nowMs) {
  ButtonEvent stopEvent = updateButton(stopButton, PIN_STOP_BUTTON, nowMs);

  if (stopEvent == BUTTON_SHORT_PRESS && alarmLatched) {
    buzzerMuteUntilMs = nowMs + BUZZER_MUTE_MS;
    Serial.println(F("Stop button short press: buzzer muted, safety remains active."));
  }

  if (stopEvent == BUTTON_LONG_PRESS && alarmLatched) {
    bool safeWindowMet = (safeSinceMs > 0) && ((nowMs - safeSinceMs) >= SAFE_WINDOW_MS);
    if (safeWindowMet) {
      resetAlarmLatches();
      currentState = isCookingContext() ? STATE_COOKING_NORMAL : STATE_NORMAL;
      Serial.println(F("Stop button long press: alarm reset after safe window."));
    } else {
      Serial.println(F("Stop button long press ignored: safe window not met."));
    }
  }

  if (USE_RESET_BUTTON) {
    ButtonEvent resetEvent = updateButton(resetButton, PIN_RESET_BUTTON, nowMs);
    if (resetEvent == BUTTON_LONG_PRESS && alarmLatched) {
      bool safeWindowMet = (safeSinceMs > 0) && ((nowMs - safeSinceMs) >= SAFE_WINDOW_MS);
      if (safeWindowMet) {
        resetAlarmLatches();
        currentState = isCookingContext() ? STATE_COOKING_NORMAL : STATE_NORMAL;
        Serial.println(F("Reset button long press: alarm reset after safe window."));
      }
    }
  }
}

void applyOutputs(unsigned long nowMs) {
  bool fanOn = false;
  bool valveAllowGas = true;
  bool buzzerOn = false;
  int servoAngle = SERVO_ANGLE_CLOSED;

  switch (currentState) {
    case STATE_WARMUP:
      fanOn = false;
      valveAllowGas = true;
      servoAngle = SERVO_ANGLE_CLOSED;
      break;

    case STATE_NORMAL:
      fanOn = false;
      valveAllowGas = true;
      servoAngle = SERVO_ANGLE_CLOSED;
      break;

    case STATE_COOKING_NORMAL:
      fanOn = false;
      valveAllowGas = true;
      servoAngle = SERVO_ANGLE_MID;
      break;

    case STATE_WARNING:
      fanOn = true;
      valveAllowGas = true;
      servoAngle = SERVO_ANGLE_MID;
      break;

    case STATE_ALARM_ACTIVE:
      fanOn = true;
      valveAllowGas = false;
      servoAngle = SERVO_ANGLE_OPEN;
      break;

    case STATE_CUTOFF_LOCKED:
      fanOn = true;
      valveAllowGas = false;
      servoAngle = SERVO_ANGLE_OPEN;
      break;
  }

  // Keep gas cutoff active while alarm is latched.
  if (alarmLatched) {
    valveAllowGas = false;
  }

  bool valveRelayOn = VALVE_ENERGIZE_TO_ALLOW_GAS ? valveAllowGas : !valveAllowGas;

  writeRelay(PIN_RELAY_FAN, fanOn);
  writeRelay(PIN_RELAY_VALVE, valveRelayOn);
  ventServo.write(servoAngle);

  bool buzzerMuted = (nowMs < buzzerMuteUntilMs);
  if ((currentState == STATE_ALARM_ACTIVE || currentState == STATE_CUTOFF_LOCKED) && !buzzerMuted) {
    buzzerOn = true;
  }
  digitalWrite(PIN_BUZZER, buzzerOn ? HIGH : LOW);

  // LEDs
  bool greenOn = false;
  bool yellowOn = false;
  bool redOn = false;

  if (currentState == STATE_NORMAL) {
    greenOn = true;
  } else if (currentState == STATE_COOKING_NORMAL) {
    if ((nowMs - lastGreenBlinkMs) >= GREEN_BLINK_MS) {
      lastGreenBlinkMs = nowMs;
      greenLedBlinkState = !greenLedBlinkState;
    }
    greenOn = greenLedBlinkState;
  } else if (currentState == STATE_WARNING) {
    yellowOn = true;
  } else if (currentState == STATE_ALARM_ACTIVE) {
    redOn = true;
  } else if (currentState == STATE_CUTOFF_LOCKED) {
    if ((nowMs - lastRedFlashMs) >= RED_FLASH_MS) {
      lastRedFlashMs = nowMs;
      redLedFlashState = !redLedFlashState;
    }
    redOn = redLedFlashState;
  }

  digitalWrite(PIN_LED_GREEN, greenOn ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, yellowOn ? HIGH : LOW);
  digitalWrite(PIN_LED_RED, redOn ? HIGH : LOW);
}

const __FlashStringHelper *stateToText(SystemState s) {
  switch (s) {
    case STATE_WARMUP:
      return F("WARMING UP");
    case STATE_NORMAL:
      return F("NORMAL");
    case STATE_COOKING_NORMAL:
      return F("COOKING NORMAL");
    case STATE_WARNING:
      return F("WARNING");
    case STATE_ALARM_ACTIVE:
      return F("ALARM ACTIVE");
    case STATE_CUTOFF_LOCKED:
      return F("CUTOFF LOCKED");
  }
  return F("UNKNOWN");
}

void updateLcd(unsigned long nowMs) {
  if ((nowMs - lastLcdMs) < LCD_UPDATE_INTERVAL_MS) {
    return;
  }
  lastLcdMs = nowMs;

  lcd.clear();

  if (currentState == STATE_WARMUP) {
    unsigned long elapsed = nowMs - startupMs;
    unsigned long remain = (elapsed >= MQ2_WARMUP_MS) ? 0 : (MQ2_WARMUP_MS - elapsed);
    lcd.setCursor(0, 0);
    lcd.print(F("WARMUP "));
    lcd.print(remain / 1000UL);
    lcd.print(F("s"));
    lcd.setCursor(0, 1);
    lcd.print(F("G:"));
    lcd.print(gasAvg);
    lcd.print(F(" B:"));
    lcd.print(gasBaseline);
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print(stateToText(currentState));

  bool buzzerMuted = (nowMs < buzzerMuteUntilMs);
  if (buzzerMuted && alarmLatched) {
    lcd.setCursor(0, 1);
    lcd.print(F("MUTED SAFE ON"));
  } else {
    lcd.setCursor(0, 1);
    lcd.print(F("G:"));
    lcd.print(gasAvg);
    lcd.print(F(" T:"));
    lcd.print(tempAvg, 1);
  }
}

void updateSerialLog(unsigned long nowMs) {
  if ((nowMs - lastSerialMs) < SERIAL_UPDATE_INTERVAL_MS) {
    return;
  }
  lastSerialMs = nowMs;

  int riskScore = computeRiskScore();

  Serial.print(F("state="));
  Serial.print((int)currentState);
  Serial.print(F(", gas="));
  Serial.print(gasAvg);
  Serial.print(F(", base="));
  Serial.print(gasBaseline);
  Serial.print(F(", dGas/s="));
  Serial.print(gasRiseCountsPerSec, 1);
  Serial.print(F(", temp="));
  Serial.print(tempAvg, 1);
  Serial.print(F(", dT/min="));
  Serial.print(tempRiseCPerMin, 2);
  Serial.print(F(", flame="));
  Serial.print(flamePersistent ? F("1") : F("0"));
  Serial.print(F(", risk="));
  Serial.print(riskScore);
  Serial.print(F(", alarmLatched="));
  Serial.print(alarmLatched ? F("1") : F("0"));
  Serial.print(F(", cutoffLocked="));
  Serial.println(cutoffLocked ? F("1") : F("0"));
}

void setup() {
  pinMode(PIN_STOP_BUTTON, INPUT_PULLUP);
  pinMode(PIN_RESET_BUTTON, INPUT_PULLUP);
  pinMode(PIN_FLAME, INPUT);

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RELAY_FAN, OUTPUT);
  pinMode(PIN_RELAY_VALVE, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  Serial.begin(9600);
  dht.begin();

  lcd.init();
  lcd.backlight();

  ventServo.attach(PIN_SERVO);
  ventServo.write(SERVO_ANGLE_CLOSED);

  // Safe defaults at boot.
  digitalWrite(PIN_BUZZER, LOW);
  writeRelay(PIN_RELAY_FAN, false);
  writeRelay(PIN_RELAY_VALVE, VALVE_ENERGIZE_TO_ALLOW_GAS ? true : false);

  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, LOW);

  startupMs = millis();

  // Prime filter buffers to avoid large transient swings.
  gasRaw = analogRead(PIN_MQ2);
  for (uint8_t i = 0; i < FILTER_WINDOW; i++) {
    pushGasSample(gasRaw);
    pushTempSample(tempC);
  }
  gasBaseline = gasAvg;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Smart Kitchen"));
  lcd.setCursor(0, 1);
  lcd.print(F("Booting..."));
  delay(1200);

  Serial.println(F("Smart Kitchen controller started."));
}

void loop() {
  unsigned long nowMs = millis();

  updateSensorReadings(nowMs);
  updateGasBaseline(nowMs);
  updateStateMachine(nowMs);
  handleButtons(nowMs);
  applyOutputs(nowMs);
  updateLcd(nowMs);
  updateSerialLog(nowMs);
}
