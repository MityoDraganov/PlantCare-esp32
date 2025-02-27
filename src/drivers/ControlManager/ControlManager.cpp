#include "drivers/ControlManager/ControlManager.h"
#include "utils/SerialManager/SerialManager.h"
#include <ArduinoJson.h>

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
        String dependentSensor = String(control->getDependentSensor().c_str());
        // Get the GPIO pin based on the dependent sensor's serial number
        int gpioPin = serialManager.getGPIOPinForSerial(dependentSensor);
        Serial.println("Initializing control for dependent sensor: " + dependentSensor);
        Serial.println("Assigned GPIO pin: " + String(gpioPin));
        control->init(gpioPin);
    }
}

// Read all controls and return their configuration as a JSON document
DynamicJsonDocument ControlManager::readAllControls()
{
    const size_t JSON_DOC_SIZE = 512; // You can adjust the size here
    DynamicJsonDocument controlData(JSON_DOC_SIZE);
    JsonArray controlArray = controlData.to<JsonArray>();

    for (Control *control : getInstance().controls)
    {
        if (control->getGpio() != -1)
        { // Check if the GPIO is initialized
            JsonObject controlObject = controlArray.createNestedObject();
            controlObject["dependentSensor"] = control->getDependentSensor();
            controlObject["ON_Value"] = control->getONValue();
            controlObject["OFF_Value"] = control->getOFFValue();
        }
        else
        {
            Serial.print("Control with dependent sensor ");
            Serial.print(control->getDependentSensor().c_str()); // Fixed here
            Serial.println(" has no initialized GPIO.");
        }
    }

    return controlData;
}
