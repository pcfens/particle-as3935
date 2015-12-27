Particle AS3935 (I2C)
=====================

A library for communicating with (and hopefully making use of) the
[AMS Franklin Lightning Sensor](http://ams.com/eng/Products/Lightning-Sensor/Franklin-Lightning-Sensor/AS3935).

The library was written and tested on a Particle Photon, but should work on an
Arduino with little to no modification.

## Usage

Reading data off of the sensor is quite simple, and is easy to get started if
you can already connect your AS3935 to your processor.

### Connecting

I've used the AS3935 breakout board by [Embedded Adventures](http://www.embeddedadventures.com/as3935_lightning_sensor_module_mod-1016.html),
though others have had success with breakout boards by other suppliers (it's all
the same sensor).

Everything required is available on the breakout board, so you only need a way
to connect your processor to the breakout board.

#### On a Particle

| AS3935 Pin     | Particle Pin                        |
| -------------: | :---------------------------------- |
| 4 (GND)        | GND                                 |
| 5 (VDD/VCC)    | 3V3                                 |
| 10 (IRQ)       | D2 (or any other interrupt pin)     |
| 11 (I2CL/SCL)  | D1 (SCL)                            |
| 13 (I2CD/SDA)  | D0 (SDA)                            |

### Software

The software on a Particle Photon is quite simple, and short enough to include
here:
```c++
#include "application.h"
#include "AS3935.h"

// Create the AS3935 object globally
AS3935::AS3935 sensor(0x00, D2);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting....");

  // Enable I2C and interrupts
  sensor.begin();

  // Calibrate the sensor, and set the value of the tuning capacitor
  sensor.calibrate(0x08);

  // Set a noise floor of 0
  sensor.setNoiseFloor(0);
}

void loop() {
  // If an interrupt triggered and is waiting for us to do something
  if(sensor.waitingInterrupt()){
    switch(sensor.getInterrupt()){
      // If there was a lightning strike
      case AS3935::INT_STRIKE:
        Serial.println("Lightning");
        break;
      // If the interrupt was triggered by a disturber, we should mask them
      case AS3935::INT_DISTURBER:
        Serial.println("Disturber - masking");
        sensor.setMaskDisturbers(true);
        break;
      // If the interrupt was caused by noise, raise the noise floor
      case AS3935::INT_NOISE:
        Serial.println("Noise");
        sensor.raiseNoiseFloor();
        break;
      // This should never execute, but we'll put it here because best practices
      default:
        break;
    }
  }
}
```

## Contributing

Feel free to send pull requests, or file bugs if you discover any. There isn't
any automated testing yet, but there hopefully will be soon.
