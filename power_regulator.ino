#include <TFT.h>

//#include <Adafruit_GFX.h>
//#include <Adafruit_PCD8544.h>
#include "TimerOne.h"

const unsigned buttonTimeoutMs = 100;
const unsigned adcTimeoutMs = 500;

float temperature = 0.;
unsigned power = 0;
unsigned long buttonPressedTimestamp = 0;
unsigned long adcTimestamp = 0;

// pin 3 - Serial clock out (SCLK)
// pin 4 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 6 - LCD chip select (CS)
// pin 7 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);

void setup()   {
  Serial.begin(9600);

  display.begin();
  display.setContrast(30); 
  
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT); 
  
  Timer1.initialize(100000); //us
  Timer1.pwm(9, 0);
  Timer1.attachInterrupt(timer1Callback);
}

void lcdPrintString(String s) {
  for (unsigned i = 0; i < s.length(); ++i) {
    display.write(s[i]);
  }
}

void timer1Callback() {
  Timer1.pwm(9, int(power / 100. * 1023.));
}

void loop() 
{
  if (millis() - buttonPressedTimestamp > buttonTimeoutMs) {
    if (digitalRead(10) == HIGH && power > 0) {
      power--;
      buttonPressedTimestamp = millis();
    }
    if (digitalRead(12) == HIGH && power < 100) {
      power++;
      buttonPressedTimestamp = millis();
    }
  }
  
  if (millis() - adcTimestamp > adcTimeoutMs) {
    temperature = analogRead(0) * 500. / 1024. - 273.15;
    adcTimestamp = millis();
  }
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(0, 0);
  lcdPrintString("T: " + String(temperature, 1) + "C");
  display.setCursor(0, 10);
  lcdPrintString("P: " + String(power) + "%");
  display.display();
}

