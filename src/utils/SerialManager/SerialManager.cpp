#include "SerialManager.h"
#include <set>


const int channelToGPIO[] = {32, 33, 34, 35};

// **Update Sensor Serial**
void SerialManager::updateSensorSerialNumber(const String &serialNumber, uint8_t channel)
{
    String cleanSerial = serialNumber;
    int typeIndex = serialNumber.indexOf("type=");
    
    // Extract only the serial part if it contains type information
    if (typeIndex != -1) {
        cleanSerial = serialNumber.substring(0, typeIndex);
    }

    _serialMap[cleanSerial] = {channel};
    knownSensorSerials.insert(cleanSerial);
}

// **Update Control Serial**
void SerialManager::updateControlSerialNumber(const String &serialNumber, uint8_t channel)
{
    String cleanSerial = serialNumber;
    int typeIndex = serialNumber.indexOf("type=");
    
    // Extract only the serial part if it contains type information
    if (typeIndex != -1) {
        cleanSerial = serialNumber.substring(0, typeIndex);
    }

    _serialMap[cleanSerial] = {channel};
    knownControlSerials.insert(cleanSerial);
}

// **Check if Serial is Sensor**
bool SerialManager::isSensorSerial(const String &serialNumber) const
{
    return knownSensorSerials.find(serialNumber) != knownSensorSerials.end();
}

// **Check if Serial is Control**
bool SerialManager::isControlSerial(const String &serialNumber) const
{
    return knownControlSerials.find(serialNumber) != knownControlSerials.end();
}

// **Check if Serial is Known**
bool SerialManager::isSerialKnown(const String &serialNumber) const
{
    return _serialMap.find(serialNumber) != _serialMap.end();
}

// **Get GPIO Pin for Serial**
int SerialManager::getGPIOPinForSerial(const String &serialNumber) const
{
    auto it = _serialMap.find(serialNumber);
    if (it != _serialMap.end())
    {
        int channel = it->second.channel;
        return channelToGPIO[channel]; // Retrieve GPIO from channel mapping
    }
    return -1; // Indicate an error if serialNumber is not found
}

// **Print All Serials**
void SerialManager::printAllSerials() const
{
    Serial.println("Current serials:");
    for (const auto &pair : _serialMap)
    {
        Serial.print("Serial: " + pair.first);
        Serial.print(", Channel: " + String(pair.second.channel));
        Serial.print(", Type: " + String(isSensorSerial(pair.first) ? "Sensor" : "Control"));
        Serial.println();
    }
}

// **Get All Serials**
std::map<String, SerialManager::SerialInfo> SerialManager::getAllSerials() const
{
    return _serialMap;
}

// **Remove Serial**
void SerialManager::removeSerialNumber(const String &serialNumber)
{
    _serialMap.erase(serialNumber);
    knownSensorSerials.erase(serialNumber);
    knownControlSerials.erase(serialNumber);
}
