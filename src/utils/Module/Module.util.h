#ifndef MODULE_UTIL_H
#define MODULE_UTIL_H

#include <Arduino.h>

class ModuleUtil
{
public:
    ModuleUtil(int firstAnalogSensorPin = 32);

    // External EEPROM methods (via I2C)
    void readModules();

private:
    int _firstAnalogSensorPin;
    static const int NUM_MUX_CHANNELS = 4;
    static const int SERIAL_NUM_LENGTH = 16;
};

#endif