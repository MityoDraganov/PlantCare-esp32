#include "EEPROM.util.h"

EEPROMUtil::EEPROMUtil(size_t size) : _size(size) {}

void EEPROMUtil::begin() {
    EEPROM.begin(_size);
}

void EEPROMUtil::writeInt(int address, int value) {
    EEPROM.write(address, (value >> 8) & 0xFF);    // High byte
    EEPROM.write(address + 1, value & 0xFF);       // Low byte
    EEPROM.commit();
}

int EEPROMUtil::readInt(int address) {
    int value = (EEPROM.read(address) << 8) | EEPROM.read(address + 1);
    return value;
}

void EEPROMUtil::writeFloat(int address, float value) {
    byte *p = (byte *)(void *)&value;
    for (int i = 0; i < sizeof(value); i++) {
        EEPROM.write(address + i, *p++);
    }
    EEPROM.commit();
}

float EEPROMUtil::readFloat(int address) {
    float value = 0.0;
    byte *p = (byte *)(void *)&value;
    for (int i = 0; i < sizeof(value); i++) {
        *p++ = EEPROM.read(address + i);
    }
    return value;
}

void EEPROMUtil::writeString(int address, const String &value, int length) {
    for (int i = 0; i < length; i++) {
        if (i < value.length()) {
            EEPROM.write(address + i, value[i]);
        } else {
            EEPROM.write(address + i, 0); // Write null character if the string is shorter than the length
        }
    }
    EEPROM.commit();
}

String EEPROMUtil::readString(int address, int length) {
    char data[length + 1];
    for (int i = 0; i < length; i++) {
        data[i] = EEPROM.read(address + i);
    }
    data[length] = '\0'; // Null-terminate the string
    return String(data);
}
