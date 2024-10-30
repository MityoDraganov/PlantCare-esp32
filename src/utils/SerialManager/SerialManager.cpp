#include "SerialManager.h"

SerialManager::SerialManager() {}

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
