#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include <map>

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

    void updateSerialNumber(const String &serialNumber, uint8_t channel);
    bool isSerialKnown(const String &serialNumber) const;
    void printAllSerials() const;
    std::map<String, SerialInfo> getAllSerials() const;
    void removeSerialNumber(const String &serialNumber);
    int getGPIOPinForSerial(const String &serialNumber) const;

private:
    SerialManager() {}
    ~SerialManager() {}
    SerialManager(const SerialManager &) = delete;
    SerialManager &operator=(const SerialManager &) = delete;

    std::map<String, SerialInfo> _serialMap;
};

#endif
