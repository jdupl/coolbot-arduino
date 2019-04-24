#include <OneWire.h>
#include <DallasTemperature.h>


#define HEATER_MOSFET_PIN   2
#define LED_PIN   13
#define ONE_WIRE_BUS 6
#define WAIT_CHANGE_MS 1000


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long lastHeaterMosfetChangeMS;
boolean heaterMosfetClosed = false;
int targetTemp = 2;

void setup() {
    Serial.begin(9600);
    pinMode(HEATER_MOSFET_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(HEATER_MOSFET_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    sensors.begin();
}

boolean canChangeState() {
  if (lastHeaterMosfetChangeMS <= 0 || lastHeaterMosfetChangeMS > WAIT_CHANGE_MS) {
    return true;
  }
  unsigned long waitMS = WAIT_CHANGE_MS - lastHeaterMosfetChangeMS;
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
    float t2 = sensors.getTempCByIndex(1); // FIXME

    Serial.println("Current t1: " + String(t1) + "C");
    Serial.println("Current t2: " + String(t2) + "C");

    if (mustOpenForIce(t1, t2)) {
      Serial.println("Mosfet should be open to relieve AC of ice !");
      if (!heaterMosfetClosed) {
        Serial.println("Mosfet already open");
      } else{
        lastHeaterMosfetChangeMS = millis();
        Serial.println("Opening mosfet now !");
        heaterMosfetClosed = false;
        digitalWrite(HEATER_MOSFET_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
      }
  } else if (mustOpenForHeater(t1, t2)) {
      Serial.println("Mosfet should be opened to cool heater");
    if (!heaterMosfetClosed) {
      Serial.println("Mosfet already opened");
    } else{
      lastHeaterMosfetChangeMS = millis();
      Serial.println("Opening mosfet now !");
      heaterMosfetClosed = false;
      digitalWrite(HEATER_MOSFET_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }
  } else if (mustCloseForRoom(t1, t2)) {
      Serial.println("Mosfet should be closed to power AC");
      if (heaterMosfetClosed) {
        Serial.println("Mosfet already closed");
      } else{
        if (canChangeState()) {
          lastHeaterMosfetChangeMS = millis();
          Serial.println("Closing mosfet now !");
          heaterMosfetClosed = true;
          digitalWrite(HEATER_MOSFET_PIN, HIGH);
          digitalWrite(LED_PIN, HIGH);
        }
      }
    } else {
      // Temp is ok, open circuit
      Serial.println("Target temperature achieved");
      if (!heaterMosfetClosed) {
        Serial.println("Mosfet already open");
      } else{
        if (canChangeState()) {
          lastHeaterMosfetChangeMS = millis();
          Serial.println("Opening mosfet now !");
          heaterMosfetClosed = false;
          digitalWrite(HEATER_MOSFET_PIN, LOW);
          digitalWrite(LED_PIN, LOW);
        }
      }
    }
    delay(250);
}
