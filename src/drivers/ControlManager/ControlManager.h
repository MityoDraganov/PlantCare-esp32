#ifndef CONTROLMANAGER_H
#define CONTROLMANAGER_H

#include <vector>
#include <Arduino.h>
#include "drivers/Control/Control.h"
#include "drivers/ControlFactory.h"
#include <ArduinoJson.h>

class ControlManager {
public:
    // Register a new control
    static void registerControl(Control* control);

    // Retrieve a control by its dependent sensor's serial number
    static Control* getControlByDependentSensor(const char* dependentSensor);

    // Get a reference to the vector containing all registered controls
    static std::vector<Control*>& getAllControls();

    // Initialize all controls (e.g., assign GPIO pins)
    static void initializeControls();

    // Read all controls into a JSON document
    static DynamicJsonDocument readAllControls();

private:
    std::vector<Control*> controls;

    // Singleton instance getter
    static ControlManager& getInstance() {
        static ControlManager instance;
        return instance;
    }
};

#endif // CONTROLMANAGER_H
