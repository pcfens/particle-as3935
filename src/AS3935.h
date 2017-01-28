#ifndef _AS3935
#define _AS3935

// Make library cross-compatiable
// with Arduino, GNU C++ for tests, and Spark.
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#elif defined(SPARK)
#include "application.h"
#endif


namespace AS3935
{
  static const int INT_STRIKE = 0x08;
  static const int INT_DISTURBER = 0x04;
  static const int INT_NOISE = 0x01;

  class AS3935
  {
    public:
        AS3935(uint8_t deviceAddress, int intPin);
        ~AS3935(void);
        void begin(void);

        void calibrate(uint8_t tunCap);

        void reset(void);

        uint8_t getInterrupt(void);

        uint8_t getDistance(void);

        uint8_t getNoiseFloor(void);
        bool setNoiseFloor(int noiseFloor);
        uint8_t raiseNoiseFloor(void);
        uint8_t lowerNoiseFloor(void);

        uint8_t getMinStrikes(void);
        bool setMinStrikes(int minStrikes);

        bool getIndoors(void);
        bool setIndoors(bool indoors);

        bool getMaskDisturbers(void);
        bool setMaskDisturbers(bool maskDisturbers);

        bool getDispLco(void);
        bool setDispLco(bool dispLco);

        bool waitingInterrupt(void);


    private:
        volatile bool _interruptWaiting;
        int _interruptPin;
        int _i2caddr;

        uint8_t _getShift(uint8_t mask);
        void _registerWrite(uint8_t reg, uint8_t mask, uint8_t value);
        void _registerWrite(uint8_t reg, uint8_t value);
        uint8_t _registerRead(uint8_t reg, uint8_t mask);
        uint8_t _rawRegisterRead(uint8_t reg);

        void _handleInterrupt(void);
  };
}

#endif
