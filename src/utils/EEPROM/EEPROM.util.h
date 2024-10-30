#ifndef EEPROM_UTIL_H
#define EEPROM_UTIL_H

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>  // For internal EEPROM (ESP32)

class EEPROMUtil {
public:
    EEPROMUtil(uint8_t eepromAddress);   // Constructor for external EEPROM

    // Initialize external EEPROM and internal EEPROM for WiFi credentials
    void begin();

    // External EEPROM methods (via I2C)
    void writeByteExternal(int address, byte data, uint8_t muxChannel);
    byte readByteExternal(int address, uint8_t muxChannel);
    void writeStringExternal(int address, const String &value, int maxLength, uint8_t muxChannel);
    String readStringExternal(int address, int maxLength, uint8_t muxChannel);
    // Internal EEPROM methods (for WiFi credentials)
    void saveWiFiCredentials(const String &ssid, const String &password);
    bool loadWiFiCredentials(String &ssid, String &password);

private:
    uint8_t _eepromAddress;  // I²C address for external EEPROMs

    // Helper methods for internal EEPROM operations
    void writeByteInternal(int address, byte data);
    byte readByteInternal(int address);
    void writeStringInternal(int address, const String &value, int maxLength);
    String readStringInternal(int address, int maxLength);

    // Internal EEPROM address offsets for Wi-Fi credentials
    const int SSID_ADDRESS = 0;         // Start address for SSID in internal EEPROM
    const int PASSWORD_ADDRESS = 100;   // Start address for Password in internal EEPROM
    const int MAX_SSID_LENGTH = 32;     // Maximum SSID length
    const int MAX_PASSWORD_LENGTH = 64; // Maximum password length
};

#endif
