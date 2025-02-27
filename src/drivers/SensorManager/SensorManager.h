#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include <vector>
#include <Arduino.h>
#include "drivers/Sensor/Sensor.h"
#include "drivers/SensorFactory.h"
#include <ArduinoJson.h>

class SensorManager {
public:
    static void registerSensor(Sensor *sensor);
    static Sensor* getSensorByType(const char *type);
    static std::vector<Sensor *> &getAllSensors();
    static void initializeSensors();
    static DynamicJsonDocument readAllSensorsAndControls();
    Sensor* getSensorBySerialNumber(const String &serialNumber);

private:
    std::vector<Sensor *> sensors;

    static SensorManager& getInstance() {
        static SensorManager instance;
        return instance;
    }
};

#endif // SENSORMANAGER_H
