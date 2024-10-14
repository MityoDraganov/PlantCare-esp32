#include "utils/EEPROM/EEPROM.util.h"
#include <Arduino.h>

EEPROMUtil::EEPROMUtil(uint8_t deviceAddress) : _deviceAddress(deviceAddress) {}

void EEPROMUtil::begin() {
    Wire.begin(25, 26); // Initialize I2C communication with custom SDA and SCL pins
}

void EEPROMUtil::writeByte(int address, byte data) {
    Wire.beginTransmission(_deviceAddress); // Begin transmission to the EEPROM
    Wire.write((address >> 8) & 0xFF); // Write high byte of address
    Wire.write(address & 0xFF);        // Write low byte of address
    Wire.write(data);                  // Write the data byte
    Wire.endTransmission();            // End transmission
    delay(5);                          // Wait for the write cycle to complete (necessary for EEPROMs)
}

byte EEPROMUtil::readByte(int address) {
    Wire.beginTransmission(_deviceAddress);
    Wire.write((address >> 8) & 0xFF); // Write high byte of address
    Wire.write(address & 0xFF);        // Write low byte of address
    Wire.endTransmission();            // End transmission

    Wire.requestFrom(_deviceAddress, 1); // Request 1 byte from the EEPROM
    return Wire.read();                 // Read and return the byte
}

void EEPROMUtil::writeInt(int address, int value) {
    writeByte(address, (value >> 8) & 0xFF);    // High byte
    writeByte(address + 1, value & 0xFF);       // Low byte
}

int EEPROMUtil::readInt(int address) {
    int value = (readByte(address) << 8) | readByte(address + 1);
    return value;
}

void EEPROMUtil::writeFloat(int address, float value) {
    byte *p = (byte *)(void *)&value;
    for (int i = 0; i < sizeof(value); i++) {
        writeByte(address + i, *p++);
    }
}

float EEPROMUtil::readFloat(int address) {
    float value = 0.0;
    byte *p = (byte *)(void *)&value;
    for (int i = 0; i < sizeof(value); i++) {
        *p++ = readByte(address + i);
    }
    return value;
}

void EEPROMUtil::writeString(int address, const String &value, int length) {
    for (int i = 0; i < length; i++) {
        if (i < value.length()) {
            writeByte(address + i, value[i]);
        } else {
            writeByte(address + i, 0); // Write null character if the string is shorter than the length
        }
    }
}

String EEPROMUtil::readString(int address, int length) {
    char data[length + 1];
    for (int i = 0; i < length; i++) {
        data[i] = readByte(address + i);
    }
    data[length] = '\0'; // Null-terminate the string
    return String(data);
}
