// lib/SensorManager/SensorManager.h
#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include <vector>
#include "drivers/Sensor/Sensor.h"
#include <string.h>
#include <Arduino.h>

class SensorManager
{
public:
    static void registerSensor(Sensor *sensor)
    {
        getInstance().sensors.push_back(sensor);
        Serial.println("Sensor registered: " + String(sensor->getType()));
    }

    static Sensor *getSensorByType(const char *type)
    {
        for (Sensor *sensor : getInstance().sensors)
        {
            if (strcmp(sensor->getType(), type) == 0)
            {
                return sensor;
            }
        }
        return nullptr;
    }

    static std::vector<Sensor *> &getAllSensors()
    {
        return getInstance().sensors;
    }

private:
    std::vector<Sensor *> sensors;

    static SensorManager &getInstance()
    {
        static SensorManager instance;
        return instance;
    }
};

#endif // SENSORMANAGER_H
