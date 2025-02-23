#include "utils///EEPROM///EEPROM.util.h"
#include "utils/Multiplexer/Multiplexer.h"

Multiplexer mux(12, 13);

EEPROMUtil::EEPROMUtil(uint8_t eepromAddress) : _eepromAddress(eepromAddress) {}

void EEPROMUtil::begin()
{
    mux.begin();
    EEPROM.begin(512);
}
void EEPROMUtil::writeByteExternal(int address, byte data, uint8_t muxChannel)
{
    mux.selectChannel(muxChannel);

    Wire.beginTransmission(0x50);
    Wire.write((int)(address >> 8));
    Wire.write((int)(address & 0xFF));
    Wire.write(data);
    Wire.endTransmission();
    delay(5); // External //EEPROM write delay (timing requirement)
}

bool isDevicePresent(uint8_t channel, uint8_t deviceAddress) {
     mux.selectChannel(channel);
    Wire.beginTransmission(deviceAddress);
    return (Wire.endTransmission() == 0);  // Returns true if device acknowledges
}


byte EEPROMUtil::readByteExternal(int address, uint8_t muxChannel) {
    if (!isDevicePresent(muxChannel, 0x50)) {
        Serial.print("No device found on MUX channel ");
        Serial.println(muxChannel);
        return 0xFF; // Indicate no valid data found
    }
    mux.selectChannel(muxChannel);
    Wire.beginTransmission(0x50);
    Wire.write((int)(address >> 8));       // MSB of address
    Wire.write((int)(address & 0xFF));     // LSB of address
    uint8_t transmissionResult = Wire.endTransmission();

    // Check for transmission error 263
    if (transmissionResult == 263) {
        return 0xFF; // Skip to next channel
    } else if (transmissionResult != 0) {
        return 0xFF; // Return error code (no valid data found)
    }

    Wire.requestFrom(0x50, 1);
    if (Wire.available()) {
        return Wire.read();
    }

    return 0xFF; // Return error code (no data available)
}

void EEPROMUtil::writeStringExternal(int address, const String &value, int maxLength, uint8_t muxChannel)
{
    int len = value.length();
    if (len > maxLength)
        len = maxLength;

    for (int i = 0; i < len; i++)
    {
        writeByteExternal(address + i, value[i], muxChannel);
    }
    for (int i = len; i < maxLength; i++)
    {
        writeByteExternal(address + i, 0, muxChannel); // Pad with null bytes
    }
}

String EEPROMUtil::readStringExternal(int address, int maxLength, uint8_t muxChannel)
{
    // Check if the device is present once at the start
    if (!isDevicePresent(muxChannel, 0x50)) {
        return "no device present on 0x50"; // Return an empty string if the device is not present
    }

    char data[maxLength + 1];
    for (int i = 0; i < maxLength; i++)
    {
        data[i] = readByteExternal(address + i, muxChannel);
    }
    data[maxLength] = '\0'; // Null-terminate the string
    return String(data);
}


//--------------- Internal //EEPROM Functions (for WiFi Credentials) --------------- //

void EEPROMUtil::writeByteInternal(int address, byte data)
{
    EEPROM.write(address, data);
    EEPROM.commit(); // Commit to internal //EEPROM
}

byte EEPROMUtil::readByteInternal(int address)
{
    return EEPROM.read(address);
}

void EEPROMUtil::writeStringInternal(int address, const String &value, int maxLength)
{
    int len = value.length();
    if (len > maxLength)
        len = maxLength;

    for (int i = 0; i < len; i++)
    {
        writeByteInternal(address + i, value[i]);
    }
    for (int i = len; i < maxLength; i++)
    {
        writeByteInternal(address + i, 0); // Pad with null bytes
    }
    EEPROM.commit();
}

String EEPROMUtil::readStringInternal(int address, int maxLength)
{
    char data[maxLength + 1];
    for (int i = 0; i < maxLength; i++)
    {
        data[i] = readByteInternal(address + i);
    }
    data[maxLength] = '\0'; // Null-terminate the string
    return String(data);
}

void EEPROMUtil::saveWiFiCredentials(const String &ssid, const String &password)
{
    writeStringInternal(SSID_ADDRESS, ssid, MAX_SSID_LENGTH);
    writeStringInternal(PASSWORD_ADDRESS, password, MAX_PASSWORD_LENGTH);
}

bool EEPROMUtil::loadWiFiCredentials(String &ssid, String &password)
{
    ssid = readStringInternal(SSID_ADDRESS, MAX_SSID_LENGTH);
    password = readStringInternal(PASSWORD_ADDRESS, MAX_PASSWORD_LENGTH);
    return ssid.length() > 0 && password.length() > 0; // Ensure valid credentials
}
