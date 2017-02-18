#include <max6675.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "TimerOne.h"
#include <PID_v1.h>

const double minGoalTemperature = 0.;
const double maxGoalTemperature = 200.;

const unsigned buttonTimeoutMs = 100;
const unsigned adcTimeoutMs = 500;

double current_temperature = 0., goal_temperature = 0.;
double power = 0; // [0..100]
unsigned long buttonPressedTimestamp = 0;
unsigned long adcTimestamp = 0;

// pin 3 - Serial clock out (SCLK)
// pin 4 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 6 - LCD chip select (CS)
// pin 7 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);

int thermoDO = A0;
int thermoCS = A1;
int thermoCLK = A2;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

double k_p = 8.17;
double k_i = 8.88;
double k_d = 1.14;

PID myPID(&current_temperature, &power, &goal_temperature, k_p, k_i, k_d, DIRECT);

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

  myPID.SetOutputLimits(0., 100.);
  myPID.SetMode(AUTOMATIC);
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
    if (digitalRead(10) == HIGH && goal_temperature > minGoalTemperature) {
      goal_temperature -= 1;
      buttonPressedTimestamp = millis();
    }
    if (digitalRead(12) == HIGH && goal_temperature < maxGoalTemperature) {
      goal_temperature += 1;
      buttonPressedTimestamp = millis();
    }
  }
  
  if (millis() - adcTimestamp > adcTimeoutMs) {
    current_temperature = thermocouple.readCelsius();
    myPID.Compute();
    adcTimestamp = millis();
  }
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  int line = 0;
  display.setCursor(0, line++ * 10);
  lcdPrintString("T_trgt: " + String(goal_temperature, 1) + "C");
  display.setCursor(0, line++ * 10);
  lcdPrintString("T_crnt: " + String(current_temperature, 1) + "C");
  display.setCursor(0, line++ * 10);
  lcdPrintString("P: " + String(power, 1) + "%");
  display.display();
}

