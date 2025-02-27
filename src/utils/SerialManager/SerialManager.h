#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include <map>
#include <set>


class SerialManager
{
public:
    struct SerialInfo
    {
        uint8_t channel;
    };

    static SerialManager &getInstance()
    {
        static SerialManager instance;
        return instance;
    }

    void updateSensorSerialNumber(const String &serialNumber, uint8_t channel);
    void updateControlSerialNumber(const String &serialNumber, uint8_t channel);
    
    bool isSensorSerial(const String &serialNumber) const;
    bool isControlSerial(const String &serialNumber) const;
    bool isSerialKnown(const String &serialNumber) const;


    void printAllSerials() const;
    std::map<String, SerialInfo> getAllSerials() const;
    void removeSerialNumber(const String &serialNumber);
    int getGPIOPinForSerial(const String &serialNumber) const;

    private:
    SerialManager() = default;
    
    std::map<String, SerialInfo> _serialMap;
    std::set<String> knownSensorSerials;
    std::set<String> knownControlSerials;
};

#endif
