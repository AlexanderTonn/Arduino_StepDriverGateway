#include <Arduino.h>
#include "arduino_stepDriverGate.hpp"

Arduino_StepDriverGate _gateway1(A0, 2, 3, 4, 400); 
Arduino_StepDriverGate _gateway2(A1, 5, 6, 7, 400);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  _gateway1.setFrequency(50);
  _gateway1.setAnalogMin(0);
  _gateway1.setAnalogMax(5000);
  _gateway1.setEnable(false); // On A4988 Enable is active low

  _gateway2.setFrequency(50);
  _gateway2.setAnalogMin(0);
  _gateway2.setAnalogMax(5000);
  _gateway2.setEnable(false); // On A4988 Enable is active low


}

void loop() {
  _gateway1.run();
  _gateway2.run();
}

