#ifndef CONTROLFACTORY_H
#define CONTROLFACTORY_H

#include <functional>
#include <map>
#include <string>
#include "drivers/Control/Control.h"  // Adjust the include path as needed

class ControlFactory {
public:
    // Define a function type for creating a Control.
    // The function takes an ON value, OFF value, and a dependent sensor (as an Arduino String),
    // and returns a pointer to a newly created Control object.
    using CreateControlFn = std::function<Control*(int onValue, int offValue, const String& dependentSensor)>;

    // Singleton instance getter.
    static ControlFactory& getInstance() {
        static ControlFactory instance;
        return instance;
    }

    // Register a control type with its corresponding creation function.
    void registerControlType(const std::string& type, CreateControlFn createFn) {
        factoryMap[type] = createFn;
    }

    // Create a control of the given type. If the type is registered,
    // the corresponding creation function is called with the provided parameters.
    Control* createControl(const std::string& type, int onValue, int offValue, const String& dependentSensor) {
        if (factoryMap.find(type) != factoryMap.end()) {
            return factoryMap[type](onValue, offValue, dependentSensor);
        }
        return nullptr;  // Unknown control type.
    }

private:
    std::map<std::string, CreateControlFn> factoryMap;
};

#endif // CONTROLFACTORY_H
