#include <set> // Add this line at the top of your file
#include "utils/Module/Module.util.h"
#include "utils/EEPROM/EEPROM.util.h"
#include "utils/SerialManager/SerialManager.h"

#include "websocket.h"

extern EEPROMUtil eepromUtil;

ModuleUtil::ModuleUtil(int firstAnalogSensorPin) : _firstAnalogSensorPin(firstAnalogSensorPin) {}

bool isValidSerialNumber(const String &serialNumber)
{
    for (char c : serialNumber)
    {
        // Check if character is not alphanumeric
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')))
        {
            return false; // Invalid character found
        }
    }
    return true; // All characters are valid
}

void ModuleUtil::readModules()
{
    std::set<String> knownSerials; // Correctly declare knownSerials
    SerialManager &serialManager = SerialManager::getInstance();

    for (int i = 0; i < NUM_MUX_CHANNELS; i++)
    {
        String serialNumber = eepromUtil.readStringExternal(0, SERIAL_NUM_LENGTH, i);

        if (serialNumber != "" && isValidSerialNumber(serialNumber))
        {

            knownSerials.insert(serialNumber); // Keep track of read serials

            if (!serialManager.isSerialKnown(serialNumber))
            {
                Serial.print("Adding serial: " + serialNumber);
                serialManager.updateSerialNumber(serialNumber, i);
                sendSensorAttachEvent(serialNumber);
            }
        } else {
            if(serialNumber != "") {
            Serial.print("Invalid serial number: " + serialNumber);
            }
        }
    }

    // Remove serial numbers that were previously known but are no longer present
    for (const auto &pair : serialManager.getAllSerials())
    {
        if (knownSerials.find(pair.first) == knownSerials.end())
        {
            Serial.print("Removing serial: ");
            Serial.println(pair.first);
            sendSensorDetachEvent(pair.first);
            serialManager.removeSerialNumber(pair.first);
        }
    }
}
