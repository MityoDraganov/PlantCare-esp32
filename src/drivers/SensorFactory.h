#ifndef SENSORFACTORY_H
#define SENSORFACTORY_H

#include <functional>
#include <map>
#include <string>
#include "drivers/Sensor/Sensor.h"

class SensorFactory {
public:
    using CreateSensorFn = std::function<Sensor*(const String& serialNumber, int gpioPin)>;

    static SensorFactory& getInstance() {
        static SensorFactory instance;
        return instance;
    }

    void registerSensorType(const std::string& type, CreateSensorFn createFn) {
        factoryMap[type] = createFn;
    }

    Sensor* createSensor(const std::string& type, const String& serialNumber, int gpioPin) {
        if (factoryMap.find(type) != factoryMap.end()) {
            return factoryMap[type](serialNumber, gpioPin);
        }
        return nullptr;  // Unknown sensor type
    }

private:
    std::map<std::string, CreateSensorFn> factoryMap;
};

#endif // SENSORFACTORY_H
