#include "drivers/ControlManager/ControlManager.h"
#include "utils/SerialManager/SerialManager.h"
#include "drivers/SensorManager/SensorManager.h"
#include <ArduinoJson.h>
#include <string>

#include "config.json.h"

extern DynamicJsonDocument jsonDoc;
const char *findKeyByControlType(const String &searchType);
const char *findDependentSensorSerialByControlType(const String &searchType);


// Register a control by adding it to the manager's list
void ControlManager::registerControl(Control *control)
{
    getInstance().controls.push_back(control);
    // Make sure dependentSensor is properly converted to String or const char* before printing
    Serial.println("Control registered with dependent sensor: " + String(control->getDependentSensor().c_str()));
}

// Retrieve a control by matching its dependent sensor serial number
Control *ControlManager::getControlByDependentSensor(const char *dependentSensor)
{
    for (Control *control : getInstance().controls)
    {
        if (control->getDependentSensor() == dependentSensor)
        { // Using operator==
            return control;
        }
    }

    // Optional debug print if no match is found
    Serial.println("No control found for dependent sensor: " + String(dependentSensor));
    return nullptr;
}

// Return the vector of all controls
std::vector<Control *> &ControlManager::getAllControls()
{
    return getInstance().controls;
}

// Initialize each control by assigning a GPIO pin (using SerialManager)

void ControlManager::initializeControls()
{
    Serial.println("Initializing controls...");
    SerialManager &serialManager = SerialManager::getInstance();
    auto serials = serialManager.getAllSerials();
    serialManager.printAllSerials();

    for (Control *control : getInstance().controls)
    {

        String controlType = String(control->getType().c_str());
        std::string serialNumber = findKeyByControlType(controlType);
        int gpioPin = serialManager.getGPIOPinForSerial(serialNumber.c_str());

        //Serial.println("Dependent Sensor Serial: " + dependentSensor);
        Serial.println("GPIO Pin Retrieved: " + String(gpioPin));


        Serial.println("Initializing control for dependent sensor: " + String(serialNumber.c_str()));
        Serial.println("Assigned GPIO pin: " + String(gpioPin));
        control->init(gpioPin);

        if (!serialNumber.empty())
        {
            int maxValue = getInstance().getMaxValueForControl(serialNumber.c_str());
            int minValue = getInstance().getMinValueForControl(serialNumber.c_str());
            control->setMaxValue(maxValue);
            control->setMinValue(minValue);
            Serial.println("Control initialized with GPIO pin: " + String(gpioPin) +
                           ", minValue: " + String(minValue) +
                           ", maxValue: " + String(maxValue));
        }
        else
        {
            Serial.println("Error: Could not find serial number for control type: " + controlType);
        }
    }
}


void ControlManager::triggerControls()
{
    //Serial.println("Triggering controls...");
    for (Control *control : getInstance().controls)
    {
        String dependentSensorSerial = findDependentSensorSerialByControlType(control->getType().c_str());
        SensorManager &sensorManager = SensorManager::getInstance();
        Sensor *linkedSensor = sensorManager.getSensorBySerialNumber(dependentSensorSerial);

        String serialNumber = findKeyByControlType(control->getType().c_str());
        SerialManager &serialManager = SerialManager::getInstance();
        int gpioPin = serialManager.getGPIOPinForSerial(serialNumber);
        Serial.println("Control GPIO Pin: " + String(gpioPin));
        pinMode(gpioPin, OUTPUT);

        if (linkedSensor != nullptr)
        {
            int sensorValue = linkedSensor->readValue();
            Serial.println("Sensor value: " + String(sensorValue));
            int minValue = control->getMinValue();
            Serial.println("Min Value: " + String(minValue));
            int maxValue = control->getMaxValue();
            Serial.println("Max Value: " + String(maxValue));

            if (sensorValue >= minValue && sensorValue <= maxValue)
            {
                Serial.println("Triggering control for sensor: " + dependentSensorSerial);
                control->start();
            }
            else
            {
                control->stop();
                Serial.println("Sensor value out of range for control: " + dependentSensorSerial);
            }
        }
        else
        {
            Serial.println("No linked sensor found for control: " + dependentSensorSerial);
        }
    }
}



const char *ControlManager::findDependentSensorSerialByControlType(const String &searchType) {
    JsonArray controls = jsonDoc["controls"].as<JsonArray>();
    for (JsonObject control : controls) {
        String type = control["type"].as<String>();
        if (type == searchType) {
            JsonObject dependantSensor = control["dependantSensor"].as<JsonObject>();
            return dependantSensor["serialNumber"].as<const char *>();
        }
    }
    return nullptr;
}

int ControlManager::getMinValueForControl(const char *serialNumber)
{

    JsonArray controls = jsonDoc["controls"];
    for (JsonObject control : controls)
    {
        if (strcmp(control["serialNumber"], serialNumber) == 0)
        {
            return control["minValue"];
        }
    }

    return -1; // Return a default value if not found
}

int ControlManager::getMaxValueForControl(const char *serialNumber)
{

    JsonArray controls = jsonDoc["controls"];
    for (JsonObject control : controls)
    {
        if (strcmp(control["serialNumber"], serialNumber) == 0)
        {
            return control["maxValue"];
        }
    }

    return -1; // Return a default value if not found
}

const char *findKeyByControlType(const String &searchType)
{
    JsonArray controls = jsonDoc["controls"].as<JsonArray>();
    for (JsonObject control : controls)
    {
        String type = control["type"].as<String>();
        if (type == searchType)
        {
            return control["serialNumber"].as<const char *>(); // Return the matching serial number
        }
    }
    return nullptr; // Return null if no match is found
}