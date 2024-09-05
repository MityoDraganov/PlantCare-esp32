#ifndef EEPROM_UTIL_H
#define EEPROM_UTIL_H

#include <EEPROM.h>

class EEPROMUtil {
public:
    EEPROMUtil(size_t size);
    void begin();
    void writeInt(int address, int value);
    int readInt(int address);
    void writeFloat(int address, float value);
    float readFloat(int address);
    void writeString(int address, const String &value, int length);
    String readString(int address, int length);

private:
    size_t _size;
};

#endif
