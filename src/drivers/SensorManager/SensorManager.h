#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include <vector>
#include <Arduino.h>
#include "drivers/Sensor/Sensor.h"
#include "drivers/SensorFactory.h"

class SensorManager {
public:
    static void registerSensor(Sensor *sensor);
    static Sensor* getSensorByType(const char *type);
    static std::vector<Sensor *> &getAllSensors();
    static void initializeSensors();
    static void readAllSensors();

private:
    std::vector<Sensor *> sensors;

    static SensorManager& getInstance() {
        static SensorManager instance;
        return instance;
    }
};

#endif // SENSORMANAGER_H
