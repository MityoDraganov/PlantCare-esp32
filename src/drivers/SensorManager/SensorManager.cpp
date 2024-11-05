#include "drivers/SensorManager/SensorManager.h"
#include "drivers/SensorFactory.h"
#include "utils/EEPROM/EEPROM.util.h"
#include "utils/SerialManager/SerialManager.h"

const int channelToGPIO[] = {32, 33, 34, 35};
extern std::map<String, String> sensorConfig;

void SensorManager::registerSensor(Sensor *sensor)
{
    getInstance().sensors.push_back(sensor);
    Serial.println("Sensor registered: " + String(sensor->getType()));
}

Sensor *SensorManager::getSensorByType(const char *type)
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

std::vector<Sensor *> &SensorManager::getAllSensors()
{
    return getInstance().sensors;
}

void SensorManager::initializeSensors()
{
    SerialManager serialManager;
    auto serials = serialManager.getAllSerials(); // Assuming this provides serials and channels

    for (const auto &[serialNumber, channel] : serials)
    {
        int gpioPin = serialManager.getGPIOPinForSerial(serialNumber);

        // Sensor *sensor = SensorFactory::getInstance().createSensor(sensorType.c_str(), serialNumber, gpioPin);
        // if (sensor)
        // {
        //     sensor->init();
        //     registerSensor(sensor);
        // }
        // else
        // {
        //     Serial.println("Unknown sensor type: " + sensorType);
        // }
    }
        for (const auto& [serialNumber, sensorType] : sensorConfig)
    {
        Serial.println("SensorConfig - Serial: " + serialNumber + ", Type: " + sensorType);
    }
}


