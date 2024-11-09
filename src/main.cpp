#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "utils/Multiplexer/Multiplexer.h"
#include "utils/EEPROM/EEPROM.util.h"
#include <random>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Arduino.h>

EEPROMUtil eepromUtil(0x50);
void generateSerialNumber(char *serial, size_t length);

const int EEPROM_SSID_ADDR = 0;
const int EEPROM_PASS_ADDR = 64;
const int EEPROM_MAX_LEN = 32;
const int NUM_MUX_CHANNELS = 4;

const int channelToGPIO[] = {32, 33, 34, 35};

DynamicJsonDocument jsonDoc(1024);

void setup()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.begin(115200);
    while (!Serial)
    {
        ;
    }

    eepromUtil.begin();
    Wire.begin();

    // Seed the random number generator with a unique value
    uint32_t seed = esp_random();
    randomSeed(seed);

    // Generate serial numbers for each mux channel
    char serialNumber[17];
    for (int channel = 0; channel < 4; ++channel)
    {
        generateSerialNumber(serialNumber, sizeof(serialNumber));
        eepromUtil.writeStringExternal(0, serialNumber, 16, channel);

        Serial.print("Written serial number for channel ");
        Serial.print(channel);
        Serial.print(": ");
        Serial.println(serialNumber);
    }

    // Read the serial numbers from EEPROM
    for (int i = 0; i < NUM_MUX_CHANNELS; i++)
    {
        String serialNumber = eepromUtil.readStringExternal(0, 16, i);
        Serial.print("Read serial number for channel ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(serialNumber);
    }
}

void loop()
{
}

void generateSerialNumber(char *serial, size_t length)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charsetSize = sizeof(charset) - 1;

    for (size_t i = 0; i < length - 1; ++i)
    {
        serial[i] = charset[random(0, charsetSize)];
    }
    serial[length - 1] = '\0';
}