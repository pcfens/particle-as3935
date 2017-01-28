#include <AS3935.h>

// Instantiate an AS3935 sensor object with the I2C address of 0x00
// and the interrupt pin connected to D2
AS3935::AS3935 sensor(0x00, D2);

void setup() {
  // Setup a serial connection and print a start banner
  Serial.begin(9600);
  Serial.println("Starting....");

  // Start I2C (if it isn't already) and attach an interrupt
  sensor.begin();

  // Write the reset command to the lightning sensor (resets calibrations and
  // other configuration state)
  sensor.reset();

  // Set the value of the tuning capacitor onthe sensor.
  sensor.calibrate(0x08);

  // Set the noise floor to 0
  sensor.setNoiseFloor(0);
}

void loop() {
  // If an interrupt is waiting to be processed let's check to see what it is
  if(sensor.waitingInterrupt()){
    // Find out what the interrupt is
    switch(sensor.getInterrupt()){

      // If it's the constant defined as INT_STRIKE, we sensed what the sensor
      // thinks is lightning.
      case AS3935::INT_STRIKE:
        Serial.println("Lightning");
        break;

      // If the interrupt code is INT_DISTURBER, we sensed a disturber.
      case AS3935::INT_DISTURBER:
        Serial.println("Disturber - masking");
        // This masks disturbers so they don't cause interrupts anymore
        sensor.setMaskDisturbers(true);
        break;

      // If the interrupt code is INT_NOISE then we just hear noise
      case AS3935::INT_NOISE:
        Serial.println("Noise");
        // Raise the noise floor to try and filter noise out. This will cause
        // the sensor to be a little less sensitive, so we want to keep it as
        // low as we reasonably can.
        sensor.raiseNoiseFloor();
        break;
        
      // This case shouldn't ever happen, but in this example we just reset
      // the interrupt and keep going to keep things simple
      default:
        break;
    }
  }
}
