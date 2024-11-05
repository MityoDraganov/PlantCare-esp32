#include "SerialManager.h"

SerialManager::SerialManager() {}
const int channelToGPIO[] = {32, 33, 34, 35};

void SerialManager::updateSerialNumber(const String& serialNumber, uint8_t channel) {
    _serialMap[serialNumber] = {channel};
}

bool SerialManager::isSerialKnown(const String& serialNumber) const {
    return _serialMap.find(serialNumber) != _serialMap.end();
}

void SerialManager::printAllSerials() const {
    Serial.println("Current serials:");
    for (const auto& pair : _serialMap) {
        Serial.print("Serial: " + pair.first);
        Serial.print(", Channel: " + String(pair.second.channel));
        Serial.println();
    }
}

std::map<String, SerialManager::SerialInfo> SerialManager::getAllSerials() const {
    return _serialMap;
}

void SerialManager::removeSerialNumber(const String& serialNumber) {
    _serialMap.erase(serialNumber);
}

int SerialManager::getGPIOPinForSerial(const String& serialNumber) const {
    auto it = _serialMap.find(serialNumber);
    if (it != _serialMap.end()) {
        int channel = it->second.channel;
        return channelToGPIO[channel];  // Retrieve GPIO from channel mapping
    }
    return -1;  // Indicate an error if serialNumber is not found
}

