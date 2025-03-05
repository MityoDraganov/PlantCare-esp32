#include <set> // For tracking known serial numbers
#include "utils/Module/Module.util.h"
#include "utils/EEPROM/EEPROM.util.h"
#include "utils/SerialManager/SerialManager.h"
#include "websocket.h"

extern EEPROMUtil eepromUtil;
extern bool isWebSocketConnected;

ModuleUtil::ModuleUtil(int firstAnalogSensorPin) : _firstAnalogSensorPin(firstAnalogSensorPin) {}

bool isValidSerialNumber(const String &serialNumber)
{
    for (char c : serialNumber)
    {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')))
        {
            return false; // Invalid character found
        }
    }
    return !serialNumber.isEmpty(); // Serial must not be empty
}

void ModuleUtil::readModules()
{
    std::set<String> knownSerials;
    SerialManager &serialManager = SerialManager::getInstance();

    for (int i = 0; i < NUM_MUX_CHANNELS; i++)
    {
        String serialNumber = eepromUtil.readStringExternal(0, SERIAL_NUM_LENGTH, i);
        String cleanSerial = serialNumber;
        int typeIndex = serialNumber.indexOf("type=");
        
        // Remove "type=" if found
        if (typeIndex != -1)
        {
            cleanSerial = serialNumber.substring(0, typeIndex);
        }

        if (isValidSerialNumber(cleanSerial))
        {
            knownSerials.insert(cleanSerial);

            if (!serialManager.isSerialKnown(cleanSerial))
            {

                // Check if it's a control or sensor
                if (serialNumber.indexOf("type=control") != -1)
                {
                    Serial.println("Control Serial Number: " + cleanSerial);
                    serialManager.updateControlSerialNumber(serialNumber, i);

                    if (isWebSocketConnected)
                    {
                        sendControlAttachEvent(cleanSerial);  // Send correct event for control
                    }
                    else
                    {
                        addPendingSerial(cleanSerial, "control");
                    }
                }
                else
                {
                    Serial.println("Sensor Serial Number: " + cleanSerial);
                    serialManager.updateSensorSerialNumber(serialNumber, i);

                    if (isWebSocketConnected)
                    {
                        sendSensorAttachEvent(cleanSerial);  // Send correct event for sensor
                    }
                    else
                    {
                        addPendingSerial(cleanSerial, "sensor");
                    }
                }
            }
        }
        else if (!serialNumber.isEmpty())
        {
            Serial.println("Invalid serial number: " + serialNumber);
        }
    }

    // Remove disconnected serial numbers
    for (const auto &pair : serialManager.getAllSerials())
    {
        if (knownSerials.find(pair.first) == knownSerials.end())
        {
            Serial.println("Removing serial: " + pair.first);
            sendSensorDetachEvent(pair.first);
            serialManager.removeSerialNumber(pair.first);
        }
    }
}

