#include "application.h"
#include "AS3935.h"

AS3935::AS3935 sensor(0x00, D2);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting....");
  sensor.begin();
  sensor.reset();
  sensor.calibrate(0x08);
  sensor.setNoiseFloor(0);
}

void loop() {
  if(sensor.waitingInterrupt()){
    switch(sensor.getInterrupt()){
      case AS3935::INT_STRIKE:
        Serial.println("Lightning");
        break;
      case AS3935::INT_DISTURBER:
        Serial.println("Disturber - masking");
        sensor.setMaskDisturbers(true);
        break;
      case AS3935::INT_NOISE:
        Serial.println("Noise");
        sensor.raiseNoiseFloor();
        break;
      default:
        break;
    }
  }
}
