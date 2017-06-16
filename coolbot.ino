#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define BTN_RIGHT  0
#define BTN_UP     1
#define BTN_DOWN   2
#define BTN_LEFT   3
#define BTN_SELECT 4
#define BTN_NONE   5
#define RELAY_PIN   6
#define ONE_WIRE_BUS 7
#define GRILL_SENSOR_INDEX 0
#define WAIT_CHANGE_MS 10000


LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long lastRelayChangeMS;
boolean relayClosed = false;
int targetTemp = 5;

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
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
    lcd.begin(16, 2);
    sensors.begin();
}

boolean canChangeState() {
  if (lastRelayChangeMS <= 0 || lastRelayChangeMS > WAIT_CHANGE_MS) {
    return true;
  }
  unsigned long waitMS = WAIT_CHANGE_MS - lastRelayChangeMS;
  Serial.println("Change avoided due to time delay. Waiting for an additionnal " + String(waitMS) + " ms.");
  return false;
}

boolean mustOpenForIce(float t1, float t2) {
  return t1 <= 2 || t2 <= 2;
}

boolean mustCloseForRoom(float t1, float t2) {
  return t1 > targetTemp || t2 > targetTemp;
}

void loop() {
    Serial.print("Requesting temperatures...");

    sensors.requestTemperatures();
    float t1 = sensors.getTempCByIndex(0);
    float t2 = sensors.getTempCByIndex(1);
    Serial.println("DONE");

    Serial.println("Current t1: " + String(t1) + "C");
    Serial.println("Current t2: " + String(t2) + "C");
    Serial.println("Target: " + String(targetTemp) + "C");

    lcd.setCursor(0,0);
    lcd.print("Target: " + String(targetTemp) + "C");
    lcd.setCursor(0,1);
    lcd.print("Current: " + String(t1) + " " + String(t2));


    if (mustOpenForIce(t1, t2)) {
      Serial.println("Relay should be open to relieve AC of ice !");
      if (!relayClosed) {
        Serial.println("Relay already open");
      } else{
        lastRelayChangeMS = millis();
        Serial.println("Opening relay now !");
        relayClosed = false;
        digitalWrite(RELAY_PIN, HIGH);
      }
    } else if (mustCloseForRoom(t1, t2)) {
      Serial.println("Relay should be closed to power AC");
      if (relayClosed) {
        Serial.println("Relay already closed");
      } else{
        if (canChangeState()) {
          lastRelayChangeMS = millis();
          Serial.println("Closing relay now !");
          relayClosed = true;
          digitalWrite(RELAY_PIN, LOW);
        }
      }
    } else {
      // Temp is ok, open circuit
      Serial.println("Target temperature achieved");
      if (!relayClosed) {
        Serial.println("Relay already open");
      } else{
        if (canChangeState()) {
          lastRelayChangeMS = millis();
          Serial.println("Opening relay now !");
          relayClosed = false;
          digitalWrite(RELAY_PIN, HIGH);
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
    delay(250);
}
