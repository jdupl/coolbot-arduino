#include <OneWire.h>
#include <DallasTemperature.h>


#define HEATER_RELAY_PIN   6
#define ONE_WIRE_BUS 7
#define WAIT_CHANGE_MS 10000


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long lastHeaterRelayChangeMS;
boolean heaterRelayClosed = false;
int targetTemp = 4;

void setup() {
    Serial.begin(9600);
    pinMode(HEATER_RELAY_PIN, OUTPUT);
    digitalWrite(HEATER_RELAY_PIN, HIGH);
    sensors.begin();
}

boolean canChangeState() {
  if (lastHeaterRelayChangeMS <= 0 || lastHeaterRelayChangeMS > WAIT_CHANGE_MS) {
    return true;
  }
  unsigned long waitMS = WAIT_CHANGE_MS - lastHeaterRelayChangeMS;
  Serial.println("Change avoided due to time delay. Waiting for an additionnal " + String(waitMS) + " ms.");
  return false;
}

boolean mustOpenForIce(float t1, float t2) {
  return t1 <= 2 || t2 <= 2;
}

boolean mustCloseForRoom(float t1, float t2) {
  return t1 > targetTemp || t2 > targetTemp;
}

boolean mustOpenForHeater(float t1, float t2) {
  return t1 > 30 || t2 > 30;
}

void loop() {
    sensors.requestTemperatures();
    float t1 = sensors.getTempCByIndex(0);
    float t2 = sensors.getTempCByIndex(0); // FIXME

    Serial.println("Current t1: " + String(t1) + "C");
    // Serial.println("Current t2: " + String(t2) + "C");

    if (mustOpenForIce(t1, t2)) {
      Serial.println("Relay should be open to relieve AC of ice !");
      if (!heaterRelayClosed) {
        Serial.println("Relay already open");
      } else{
        lastHeaterRelayChangeMS = millis();
        Serial.println("Opening relay now !");
        heaterRelayClosed = false;
        digitalWrite(HEATER_RELAY_PIN, HIGH);
      }
  } else if (mustOpenForHeater(t1, t2)) {
      Serial.println("Relay should be opened to cool heater");
    if (!heaterRelayClosed) {
      Serial.println("Relay already opened");
    } else{
      lastHeaterRelayChangeMS = millis();
      Serial.println("Opening relay now !");
      heaterRelayClosed = false;
      digitalWrite(HEATER_RELAY_PIN, HIGH);
    }
  } else if (mustCloseForRoom(t1, t2)) {
      Serial.println("Relay should be closed to power AC");
      if (heaterRelayClosed) {
        Serial.println("Relay already closed");
      } else{
        if (canChangeState()) {
          lastHeaterRelayChangeMS = millis();
          Serial.println("Closing relay now !");
          heaterRelayClosed = true;
          digitalWrite(HEATER_RELAY_PIN, LOW);
        }
      }
    } else {
      // Temp is ok, open circuit
      Serial.println("Target temperature achieved");
      if (!heaterRelayClosed) {
        Serial.println("Relay already open");
      } else{
        if (canChangeState()) {
          lastHeaterRelayChangeMS = millis();
          Serial.println("Opening relay now !");
          heaterRelayClosed = false;
          digitalWrite(HEATER_RELAY_PIN, HIGH);
        }
      }
    }
    delay(250);
}
