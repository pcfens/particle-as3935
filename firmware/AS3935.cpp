#include "AS3935.h"

/**
 * Constructor for AS3935 - sets I2C address and interrupt pin
 * @param deviceAddress The I2C address for the AS3935 detector
 * @param intPin The interrupt pin off of the AS3935
 */
AS3935::AS3935::AS3935(uint8_t deviceAddress, int intPin)
{
  _i2caddr = deviceAddress;
  _interruptPin = intPin;
}

/**
 * Class destructor that detaches the interrupt
 */
AS3935::AS3935::~AS3935()
{
  detachInterrupt(_interruptPin);
}

/**
 * Begin using the already instantiated AS3935 object
 * - Starts the I2C interface if it isn't already started
 * - Sets the interrupt pin as an input (pulldown)
 * - Attaches an interrupt to the interrupt pin
 */
void AS3935::AS3935::begin()
{
  if(!Wire.isEnabled()){
    Wire.begin();
  }
  pinMode(_interruptPin, INPUT_PULLDOWN);
   // Don't display anything on the interrupt pin
  _registerWrite(0x08, 0xE0, 0x00);

  attachInterrupt(_interruptPin, &AS3935::_handleInterrupt, this, RISING);
  _interruptWaiting = false;
}

/**
 * Calibrate the AS3935 (interrupts are disabled while calibrating)
 * @param tunCap The value of the tuning capacitor register (0-15) used to set
 * the internal tuning capacitor (0-120pF in 8pF steps)
 */
void AS3935::AS3935::calibrate(uint8_t tunCap)
{
  if(tunCap < 0x10 && tunCap >= 0){
    noInterrupts();
    _registerWrite(0x08, 0x0F, tunCap);
    delay(2);
    _registerWrite(0x3D, 0x96);
    delay(2);
    _registerWrite(0x08, 0x20, 1);
    delay(2);
    _registerWrite(0x08, 0x20, 0);
    interrupts();
  }
}

/**
 * Read a register from the AS3935 over I2C without modifying the response based
 * on the mask.
 * @param reg The register to read
 * @return An unsigned 8-bit integer representing the contents of the register
 */
uint8_t AS3935::AS3935::_rawRegisterRead(uint8_t reg)
{
  Wire.beginTransmission(_i2caddr);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(_i2caddr, 1);
  uint8_t c = Wire.read();

  return c;
}

/**
 * Read a register from the AS3935 over I2C, and return a masked and shifted
 * value
 * @param reg The register to read from
 * @param mask The mask to use when shifting contents
 * @return An unsigned 8-bit integer with the right most bits containing
 * the masked and shifted contents of the requested register
 */
uint8_t AS3935::AS3935::_registerRead(uint8_t reg, uint8_t mask)
{
  uint8_t rawReg = _rawRegisterRead(reg) & mask;
  return (rawReg >> _getShift(mask));
}

/**
 * Find the shift required to make the mask use the LSB.
 * Ex. reg=0xF0 returns 4 because 0xF0 >> 4 returns 0x0F.
 * Inspired by https://github.com/raivisr/AS3935-Arduino-Library
 * @param mask The mask to find the shift of
 * @return The number of bit positions to shift the mask
 */
uint8_t AS3935::AS3935::_getShift(uint8_t mask)
{
	uint8_t i = 0;
	for (i = 0; ~mask & 1; i++)
		mask >>= 1;
	return i;
}

/**
 * Write a full byte to a register over I2C
 * @param reg The AS3935 register to write to
 * @param value The value to write to the register (all 8 bits are written)
 */
void AS3935::AS3935::_registerWrite(uint8_t reg, uint8_t value)
{
  _registerWrite(reg, 0xFF, value);
}

/**
 * Write a masked value to the AS3935 over I2C
 * @param reg The AS3935 register to write to
 * @param mask The bitmask to write to (other bits in the register are
 * preserved)
 * @param value The value to write (should fit within mask)
 */
void AS3935::AS3935::_registerWrite(uint8_t reg, uint8_t mask, uint8_t value)
{
  uint8_t registerValue = _rawRegisterRead(reg);
  registerValue &= ~(mask);
  registerValue |= ((value << (_getShift(mask))) & mask);

  Wire.beginTransmission(_i2caddr);
  Wire.write(reg);
  Wire.write(registerValue);
  Wire.endTransmission();
}

/**
 * Reset the AS3935 to powerup defaults
 */
void AS3935::AS3935::reset(void)
{
  _registerWrite(0x3C, 0x96);
}

/**
 * Get the value of the interrupt register on the AS3935
 * @return A number representing the cause of the interrupt.
 */
uint8_t AS3935::AS3935::getInterrupt(void)
{
  _interruptWaiting = false;
  delay(3);
  uint8_t reg = _registerRead(0x03, 0x0F);
  return reg;
}

/**
 * The ISR for interrupts sent by the detector. Simply sets a flag for the
 * main service loop to detect when it's ready.
 */
void AS3935::AS3935::_handleInterrupt(void)
{
  _interruptWaiting = true;
}

/**
 * Find the estimated distance to the most recently detected lightning strike.
 * @return The estimated distance (in kilometers) to the most recently detected
 * strike. A value of 0x3F means out of range.
 */
uint8_t AS3935::AS3935::getDistance(void)
{
  return _registerRead(0x07, 0x3F);
}

/**
 * Get the noise floor from the AS3935
 * @return The current noise floor setting
 */
uint8_t AS3935::AS3935::getNoiseFloor(void)
{
  return _registerRead(0x01, 0x70);
}

/**
 * Set the sensor noise floor
 * @param noiseFloor the noise floor that should be set (ranges from 0-7).
 * Actual signal levels are available in table 16 of the data sheet.
 * @return A boolean value indicating success of the write (verified with a
 * subsequent read)
 */
bool AS3935::AS3935::setNoiseFloor(int noiseFloor)
{
  if(noiseFloor > 7 || noiseFloor < 0)
    return false;
  _registerWrite(0x01, 0x70, noiseFloor);
  return getNoiseFloor() == noiseFloor;
}

/**
 * Raise the sensor noise floor
 * @return The new noise floor
 * @warning The noise floor may not actually change if it's already at its high
 * limit of 7.
 */
uint8_t AS3935::AS3935::raiseNoiseFloor(void)
{
  uint8_t current = getNoiseFloor();
  current++;
  setNoiseFloor(current);
  return getNoiseFloor();
}

/**
 * Lower the sensor noise floor
 * @return The new noise floor
 * @warning The noise floor may not actually change if it's already at its low
 * limit of 0.
 */
uint8_t AS3935::AS3935::lowerNoiseFloor(void)
{
  uint8_t current = getNoiseFloor();
  current--;
  setNoiseFloor(current);
  return getNoiseFloor();
}

/**
 * Get the minimum number of strikes that have to be detected before an
 * interrupt is triggered by the detector.
 * @return The minimum number of strikes to trigger an interrupt (255 indicates
 * an error).
 */
uint8_t AS3935::AS3935::getMinStrikes(void)
{
  switch(_registerRead(0x02, 0xCF)){
    case 0:
      return 1;
      break;
    case 1:
      return 5;
      break;
    case 2:
      return 9;
      break;
    case 3:
      return 16;
      break;
    default:
      return 255;
      break;
  }
}

/**
 * Set the number of detected strikes that cause the detector to trigger an
 * interrupt.
 * @param minStrikes The number of strikes that trigger an interrupt. Valid
 * values are 1, 5, 9, and 16 (limited by the AS3935).
 * @return A boolean indicating success verified by a subsequent read
 */
bool AS3935::AS3935::setMinStrikes(int minStrikes)
{
  uint8_t regValue;
  switch(minStrikes){
    case 1:
      regValue = 0;
      break;
    case 5:
      regValue = 1;
      break;
    case 9:
      regValue = 2;
      break;
    case 16:
      regValue = 3;
      break;
    default:
      return false;
      break;
    }

  _registerWrite(0x02, 0xCF, regValue);
  return getMinStrikes() == regValue;
}

/**
 * Determine whether or not the gain setting is set for indoor operation
 * as specified by Table 15 on the datasheet.
 * @param true if optimized for indoor use, otherwise false
 */
bool AS3935::AS3935::getIndoors(void)
{
  return (bool)_registerRead(0x00, 0x20);
}

/**
 * Set the gain optimization for indoors or outdoors
 * @param indoors Boolean true if optimized for indoors, false to optimize for
 * outdoor use (as specified in Table 15 of the datasheet).
 * @return A boolean indicating success as verified by a subsequent read.
 */
bool AS3935::AS3935::setIndoors(bool indoors)
{
  if(indoors){
    _registerWrite(0x00, 0xC1, 0x12);
  } else {
    _registerWrite(0x00, 0xC1, 0x0E);
  }
  return getIndoors() == indoors;
}

/**
 * Find whether or not disturbers are masked and hidden by the sensor.
 * @return Boolean true if disturbers are masked, false otherwise.
 */
bool AS3935::AS3935::getMaskDisturbers(void)
{
  return (bool)_registerRead(0x03, 0x20);
}

/**
 * Mask or unmask distubers from triggering interrupts
 * @param maskDisturbers A boolean value, true to mask distubers, false to
 * unmask.
 * @return A boolean indicating success as verified by a subsequent read
 */
bool AS3935::AS3935::setMaskDisturbers(bool maskDisturbers)
{
  _registerWrite(0x03, 0x20, maskDisturbers);
  return (bool)_registerRead(0x03, 0x20) == maskDisturbers;
}

/**
 * Determine if the local oscillator (LCO) is displayed on the interrupt pin
 * @return Boolean true if the oscillator is displayed, false otherwise
 */
bool AS3935::AS3935::getDispLco(void)
{
  return (bool)_registerRead(0x08, 0x80);
}

/**
 * Display the local oscillator (LCO) on the interrupt pin, or set it to normal
 * operation.
 * @param dispLco Boolean true to display the LCO, false for normal operation
 * @return A boolean indicating success as verified by a subsequent read.
 * @warning Displaying the LCO on the interrupt pin with interrups enabled
 * can overload the system by calling the ISR several thousand times per second.
 * Consider disabling interrupts, or call this without calling AS3935::begin().
 */
bool AS3935::AS3935::setDispLco(bool dispLco)
{
  _registerWrite(0x08, 0x80, dispLco);
  return getDispLco() == dispLco;
}

/**
 * Determine if an interrupt is waiting to be handled by the normal program
 * loop
 * @return Boolean true if an interrupt occured and needs to be dealt with,
 * false otherwise.
 */
bool AS3935::AS3935::waitingInterrupt()
{
  return _interruptWaiting;
}
