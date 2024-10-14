#ifndef EEPROM_UTIL_H
#define EEPROM_UTIL_H

#include <Wire.h> // Include the Wire library for I2C
#include <Arduino.h>

class EEPROMUtil {
public:
    EEPROMUtil(uint8_t deviceAddress); // Constructor to accept the I2C address
    void begin();                        // Initialize I2C communication
    void writeInt(int address, int value);
    int readInt(int address);
    void writeFloat(int address, float value);
    float readFloat(int address);
    void writeString(int address, const String &value, int length);
    String readString(int address, int length);

private:
    uint8_t _deviceAddress; // Address of the external EEPROM
    void writeByte(int address, byte data); // Helper function to write a byte
    byte readByte(int address);              // Helper function to read a byte
};

#endif
