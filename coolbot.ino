#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

int readButtons() {
    int adc_key_in = analogRead(0);
    if (adc_key_in > 1000) return btnNONE;
    if (adc_key_in < 150) return btnRIGHT;
    if (adc_key_in < 250) return btnUP;
    if (adc_key_in < 450) return btnDOWN;
    if (adc_key_in < 650) return btnLEFT;
    if (adc_key_in < 850) return btnSELECT;
    return btnNONE;
}

void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2);
}

void loop() {
    int targetTemp = 25;
    lcd.setCursor(0,0);
    lcd.print("Target: " + String(targetTemp) + "C");
    lcd.setCursor(0,1);
    lcd.print("Current: ");
    int key = readButtons();

    if (key == btnUP) {
        targetTemp += 1;
    } else if (key == btnDOWN) {
        targetTemp -= 1;
    }
    delay(250);
}
