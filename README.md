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
#include <AS3935.h>

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

## Reference

### `AS3935`

`AS3935::AS3935 sensor(0x00, D2);`

Instantiate an instance of AS3935 to interact with a sensor.

Arguments are the I2C address and the pin where the interrupt pin is connected.

### `begin`

`sensor.begin();`

Enables I2C and sets up the interrupt routine. This is normally called in your
setup() routine.

### `calibrate`

`sensor.calibrate(0x08);`

Pass one argument (an unsigned integer) representing the value to set the
tuning capacitor to.

### `reset`

`sensor.reset();`

Reset all of the sensor settings as though it was just powered up.

### `getInterrupt`

`reason = sensor.getInterrupt();`

Returns an unsigned integer representing the reason an interrupt was triggered.
Calling this method resets `waitingInterrupt()` to false. After calling, the
value returned is not available again until an interrupt is read.

Returned values can be compared to constants (reference below) to easily
determine what caused the interrupt.

### `getDistance`

`distance = sensor.getDistance();`

Returns an unsigned integer with the estimated distance to the lightning strike

### `getNoiseFloor`

`noisefloor = sensor.getNoiseFloor();`

Returns an unsigned integer representing the current noise floor.

### `setNoiseFloor`

`sensor.setNoiseFloor(2);`

Pass one unsigned integer (ranging 0-7) as an argument representing the noise
floor to set.

Returns a boolean value indicating success or failure.

### `raiseNoiseFloor`

`sensor.raiseNoiseFloor();`

Raise the noise floor by one increment. Returns the new noise floor as an
unsigned integer ranging from 0-7. If the noise floor is 7 before calling this,
nothing will happen and it will return 7.

### `lowerNoiseFloor`

`sensor.lowerNoiseFloor();`

Lower the noise floor by one increment. Returns the new noise floor as an
unsigned integer ranging from 0-7. If the noise floor is 0 before calling this,
nothing will happen and it will return 0.

### `getMinStrikes`

`minStrikes = sensor.getMinStrikes();`

Get the minimum number of strikes that must be sensed before an interrupt is
raised. A value of 255 indicates an error.

### `setMinStrikes`

`sensor.setMinStrikes(5);`

Set the minimum number of detected lightning strikes required to trigger an
interrupt. Valid values are 1, 5, 9, or 16. Returns boolean true if the
operation is successful.

### `getIndoors`

`indoors = sensor.getIndoors();`

Determine if the sensor is configured as indoors or not. Returns boolean
true if it's configured as indoors.

### `setIndoors`

`sensor.setIndoors(true);`

Pass boolean true to set the sensor as being indoors, false for outdoors.
Returns true if successful.

### `getMaskDisturbers`

`distrubersMasked = sensor.getMaskDisturbers();`

Returns boolean true if disturbers are masked, false if they aren't.

### `setMaskDisturbers`

`sensor.setMaskDisturbers(true);`

Pass boolean true to mask disturbers, false to unmask disturbers. Returns
boolean true if successful.

### `getDispLco`

`dispLCO = sensor.getDispLco();`

Returns boolean true if the local oscillator is exposed on the interrupt pin.

### `setDispLco`

`sensor.setDispLco(false);`

Pass boolean true to expose the local oscillator on the interrupt pin. This
should only be used for tuning and troubleshooting with some sort of
instrumentation connected to the interrupt pin.

### `waitingInterrupt`

`interruptWaiting = sensor.waitingInterrupt();`

Returns true if an interrupt is waiting to be read. It can be reset to false
only by calling `getInterrupt()`.

### Constants

`AS3935::INT_STRIKE`: The value returned when the sensor detects a lightning strike.
`AS3935::INT_DISTURBER`: The value returned when the sensor detects a disturber.
`AS3935::INT_NOISE`: The value returned when the sensor detects noise.

## Contributing

Feel free to send pull requests, or file bugs if you discover any. There isn't
any automated testing yet, but there hopefully will be soon.
