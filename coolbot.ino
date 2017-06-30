#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define BTN_RIGHT  0
#define BTN_UP     1
#define BTN_DOWN   2
#define BTN_LEFT   3
#define BTN_SELECT 4
#define BTN_NONE   5
#define HEATER_RELAY_PIN   6
#define ONE_WIRE_BUS 7
#define AC_RELAY_PIN 8
#define GRILL_SENSOR_INDEX 0
#define WAIT_CHANGE_MS 10000


LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long lastHeaterRelayChangeMS;
unsigned long lastACRelayChangeMS;
boolean heaterRelayClosed = false;
unsigned long tick;
int targetTemp = 6;

int readButtons() {
    int adc_key_in = analogRead(0);
    if (adc_key_in > 1000) return BTN_NONE;
    if (adc_key_in < 150) return BTN_RIGHT;
    if (adc_key_in < 250) return BTN_UP;
    if (adc_key_in < 450) return BTN_DOWN;
    if (adc_key_in < 650) return BTN_LEFT;
    if (adc_key_in < 850) return BTN_SELECT;
    return BTN_NONE;
}

void setup() {
    Serial.begin(9600);
    pinMode(HEATER_RELAY_PIN, OUTPUT);
    pinMode(AC_RELAY_PIN, OUTPUT);
    digitalWrite(HEATER_RELAY_PIN, HIGH);
    digitalWrite(AC_RELAY_PIN, LOW);
    lcd.begin(16, 2);
    sensors.begin();
    lastACRelayChangeMS = 0;
    tick = 0;
}

boolean canChangeState() {
  if (lastHeaterRelayChangeMS <= 0 || lastHeaterRelayChangeMS > WAIT_CHANGE_MS) {
    return true;
  }
  unsigned long waitMS = WAIT_CHANGE_MS - lastHeaterRelayChangeMS;
  Serial.println("Change avoided due to time delay. Waiting for an additionnal " + String(waitMS) + " ms.");
  return false;
}

boolean mustOpenForIce(float t1, float t2, float t3) {
  return t1 <= 2 || t2 <= 2 || t3 <= 2;
}

boolean mustCloseForRoom(float t1, float t2, float t3) {
  return t1 > targetTemp || t2 > targetTemp || t3 > targetTemp;
}

boolean mustOpenForHeater(float t1, float t2, float t3) {
  return t1 > 30 || t2 > 30 || t3 > 30;
}

void triggerReset() {
  lastACRelayChangeMS = millis();
  Serial.println("RESET AC");
  tick = 0;
  digitalWrite(HEATER_RELAY_PIN, HIGH);
  delay(5000);
  digitalWrite(HEATER_RELAY_PIN, LOW);
}

boolean shouldTriggerResetAC(float t1, float t2) {
  unsigned long waitACTicks = 2400;

  float maxTempToReset = targetTemp + 2;
  if (t1 > maxTempToReset && t2 > maxTempToReset) {
    if (tick >= waitACTicks) {
      return true;
    } else {
      // Serial.println("Should reset AC, but it's gotta wait =(");
      // Serial.println("Reset " + String(resetACLastMS) + " ms ago");
      return false;
    }
  }
  return false;
}

void loop() {
    sensors.requestTemperatures();
    float t1 = sensors.getTempCByIndex(0);
    float t2 = sensors.getTempCByIndex(1);
    float t3 = sensors.getTempCByIndex(2);

    Serial.println("Current t1: " + String(t1) + "C");
    Serial.println("Current t2: " + String(t2) + "C");
    Serial.println("Current t3: " + String(t3) + "C");

    lcd.setCursor(0,0);
    lcd.print("Target: " + String(targetTemp) + "C");
    lcd.setCursor(0,1);
    lcd.print("Current: " + String(t1) + " " + String(t2));

    if (shouldTriggerResetAC(t1, t2)) {
        triggerReset();
    }

    if (mustOpenForIce(t1, t2, t3)) {
      // Serial.println("Relay should be open to relieve AC of ice !");
      if (!heaterRelayClosed) {
        // Serial.println("Relay already open");
      } else{
        lastHeaterRelayChangeMS = millis();
        // Serial.println("Opening relay now !");
        heaterRelayClosed = false;
        digitalWrite(HEATER_RELAY_PIN, HIGH);
      }
  } else if (mustOpenForHeater(t1,t2,t3)) {
    // Serial.println("Relay should be opened to cool heater");
    if (!heaterRelayClosed) {
      // Serial.println("Relay already opened");
    } else{
      lastHeaterRelayChangeMS = millis();
      // Serial.println("Opening relay now !");
      heaterRelayClosed = false;
      digitalWrite(HEATER_RELAY_PIN, HIGH);
    }
  } else if (mustCloseForRoom(t1, t2, t3)) {
      // Serial.println("Relay should be closed to power AC");
      if (heaterRelayClosed) {
        // Serial.println("Relay already closed");
      } else{
        if (canChangeState()) {
          lastHeaterRelayChangeMS = millis();
          // Serial.println("Closing relay now !");
          heaterRelayClosed = true;
          digitalWrite(HEATER_RELAY_PIN, LOW);
        }
      }
    } else {
      // Temp is ok, open circuit
      Serial.println("Target temperature achieved");
      if (!heaterRelayClosed) {
        // Serial.println("Relay already open");
      } else{
        if (canChangeState()) {
          lastHeaterRelayChangeMS = millis();
          // Serial.println("Opening relay now !");
          heaterRelayClosed = false;
          digitalWrite(HEATER_RELAY_PIN, HIGH);
        }
      }
    }
    // int key = readButtons();
    //
    // if (key == BTN_UP) {
    //     targetTemp += 1;
    // } else if (key == BTN_DOWN) {
    //     targetTemp -= 1;
    // }
    tick += 1;
    delay(250);
}
