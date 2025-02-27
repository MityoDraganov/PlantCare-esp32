#include "drivers/SensorManager/SensorManager.h"
#include "drivers/SensorFactory.h"
#include "utils/EEPROM/EEPROM.util.h"
#include "utils/SerialManager/SerialManager.h"
#include <ArduinoJson.h>

const int channelToGPIO[] = {32, 33, 34, 35};
extern std::map<String, String> sensorConfig;
extern DynamicJsonDocument jsonDoc;

const char *findKeyByValue(const String &searchValue)
{
    for (JsonPair kv : jsonDoc.as<JsonObject>())
    {
        // If the value matches the search value
        if (kv.value().as<String>() == searchValue)
        {
            return kv.key().c_str(); // Return the key as a C-string
        }
    }
    return nullptr; // Return null if no match is found
}

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
    SerialManager &serialManager = SerialManager::getInstance();
    auto serials = serialManager.getAllSerials();
    serialManager.printAllSerials();

    for (Sensor *sensor : SensorManager::getAllSensors())
    {
        Serial.println("Found sensor: " + String(sensor->getType()));
        String sensorType = sensor->getType();

        String serialNumber = findKeyByValue(sensorType);
        Serial.println("serialNumber");
        Serial.println(serialNumber);

        int gpioPin = serialManager.getGPIOPinForSerial(serialNumber);

        Serial.println("gpioPin");
        Serial.println(gpioPin);

        sensor->init(gpioPin);

        Serial.println("Sensor value: " + String(sensor->readValue()));

        registerSensor(sensor);
    }
}

// Modified to handle both sensors and controls
DynamicJsonDocument SensorManager::readAllSensorsAndControls()
{
    // Create a JSON document to store sensor and control data
    DynamicJsonDocument sensorData(512);
    JsonArray sensorArray = sensorData.createNestedArray("sensors");
    JsonArray controlArray = sensorData.createNestedArray("controls");

    for (Sensor *sensor : getInstance().sensors)
    {
        if (sensor->getGpio() != -1)
        {                                    // Check if the GPIO is initialized
            int value = sensor->readValue(); // Read the sensor value
            String serialNumber = findKeyByValue(sensor->getType());

            // Add sensor data to JSON array
            if (!serialNumber.isEmpty())
            {
                JsonObject sensorObject = sensorArray.createNestedObject();
                sensorObject["sensorSerialNumber"] = serialNumber;
                sensorObject["value"] = value;
            }
            else
            {
                Serial.print("Warning: Serial number for sensor ");
                Serial.print(sensor->getType());
                Serial.println(" not found.");
            }
        }
        else
        {
            Serial.print("Sensor Type: ");
            Serial.print(sensor->getType());
            Serial.println(" | GPIO not initialized.");
        }
    }

    // Process controls
    JsonArray controlsArray = jsonDoc["controls"].as<JsonArray>();
    for (JsonObject control : controlsArray)
    {
        String controlSerialNumber = control["serialNumber"].as<String>();
        String controlType = control["type"].as<String>();
        JsonObject dependantSensor = control["dependantSensor"].as<JsonObject>();
        
        String sensorSerial = dependantSensor["serialNumber"].as<String>();
        int minValue = dependantSensor["minValue"].as<int>();
        int maxValue = dependantSensor["maxValue"].as<int>();

        Sensor *linkedSensor = SensorManager::getInstance().getSensorBySerialNumber(sensorSerial);



        if (linkedSensor != nullptr)
        {
            int sensorValue = linkedSensor->readValue();
            JsonObject controlObject = controlArray.createNestedObject();
            controlObject["controlSerialNumber"] = controlSerialNumber;
            controlObject["type"] = controlType;
            controlObject["sensorSerialNumber"] = sensorSerial;
            controlObject["sensorValue"] = sensorValue;
            controlObject["minValue"] = minValue;
            controlObject["maxValue"] = maxValue;

            // Trigger control logic (e.g., activate/deactivate water pump)
            if (sensorValue < minValue || sensorValue > maxValue)
            {
                // Example control action, such as turning on a water pump
                Serial.println("Control triggered: " + controlType);
                // Add logic to trigger the control here (e.g., activate GPIO pin for pump)
            }
        }
    }

    return sensorData;
}

Sensor* SensorManager::getSensorBySerialNumber(const String &serialNumber)
{
    for (Sensor *sensor : getInstance().sensors)
    {
        String sensorSerial = findKeyByValue(sensor->getType());
        if (sensorSerial == serialNumber)
        {
            return sensor;
        }
    }
    return nullptr;
}
