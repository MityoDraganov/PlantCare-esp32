#ifndef MODULE_UTIL_H
#define MODULE_UTIL_H

#include <Arduino.h>

class ModuleUtil
{
public:
    ModuleUtil(int firstAnalogSensorPin = 32);
    void readModules(); // Reads and manages attached modules

private:
    int _firstAnalogSensorPin;
    static const int NUM_MUX_CHANNELS = 4;
    static const int SERIAL_NUM_LENGTH = 32;
};

#endif // MODULE_UTIL_H
