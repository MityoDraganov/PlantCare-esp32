#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "utils/Multiplexer/Multiplexer.h"
#include <ArduinoOTA.h>
#include "utils/EEPROM/EEPROM.util.h"
#include <random>
#include "utils/Module/Module.util.h"
#include <Wire.h>
#include "drivers/SensorManager/SensorManager.h"
#include "config.json.h"
#include <Ticker.h>

EEPROMUtil eepromUtil(0x50);
ModuleUtil moduleUtil(32);
String generateSerialNumber(int length);

const int EEPROM_SSID_ADDR = 0;
const int EEPROM_PASS_ADDR = 64;
const int EEPROM_MAX_LEN = 32;
const int NUM_MUX_CHANNELS = 4;

const int channelToGPIO[] = {32, 33, 34, 35};

DynamicJsonDocument jsonDoc(1024);

void setup()
{
    if (!SPIFFS.begin(true))
    { // Pass true to format if failed to mount
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

    // Generate serial numbers for each mux channel
    for (int i = 0; i < NUM_MUX_CHANNELS; i++)
    {
        String serialNumber = generateSerialNumber(16);
        eepromUtil.writeStringExternal(0, serialNumber, 16, i);
    }

    // Read the serial numbers from EEPROM
    for (int i = 0; i < NUM_MUX_CHANNELS; i++)
    {
        String serialNumber = eepromUtil.readStringExternal(0, 16, i);
        Serial.print("Serial number for channel ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(serialNumber);
    }
}

void loop()
{
}

std::mt19937 generator;
String generateSerialNumber(int length)
{
    const String characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    String serialNumber;

    // Use the global random generator (already seeded in setup)
    std::uniform_int_distribution<> distribution(0, characters.length() - 1);

    for (int i = 0; i < length; ++i)
    {
        serialNumber += characters[distribution(generator)];
    }

    return serialNumber;
}