#include <set> // Add this line at the top of your file
#include "utils/Module/Module.util.h"
#include "utils/EEPROM/EEPROM.util.h"
#include "utils/SerialManager/SerialManager.h"

EEPROMUtil eepromUtil(0x50);
SerialManager serialManager; // Corrected declaration

ModuleUtil::ModuleUtil(int firstAnalogSensorPin) : _firstAnalogSensorPin(firstAnalogSensorPin) {}

void ModuleUtil::readModules()
{
    std::set<String> knownSerials; // Correctly declare knownSerials
    
    for (int i = 0; i < NUM_MUX_CHANNELS; i++)
    {
        String serialNumber = eepromUtil.readStringExternal(0, SERIAL_NUM_LENGTH, i);
       

        if (serialNumber != "")
        {
            Serial.print("Read serial for Channel ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(serialNumber);

            knownSerials.insert(serialNumber); // Keep track of read serials
            
            // If the serial number is new, add it to the SerialManager
            if (!serialManager.isSerialKnown(serialNumber))
            {
                serialManager.updateSerialNumber(serialNumber, i);
                Serial.print("Added new serial: ");
                Serial.println(serialNumber);
            }
        }
    }

    // Remove serial numbers that were previously known but are no longer present
    for (const auto& pair : serialManager.getAllSerials())
    {
        if (knownSerials.find(pair.first) == knownSerials.end())
        {
            // If the serial number is not found in knownSerials, remove it
            Serial.print("Removing serial: ");
            Serial.println(pair.first);
            serialManager.removeSerialNumber(pair.first);
        }
    }
}
