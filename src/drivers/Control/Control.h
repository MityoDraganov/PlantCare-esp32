#ifndef CONTROL_H
#define CONTROL_H

#include <string>

class Control
{
public:
    virtual void init(int gpioPin) = 0;

    // To get the ON value for the control
    virtual int getONValue() const = 0;

    // To get the OFF value for the control
    virtual int getOFFValue() const = 0;

    // To get the dependent sensor's serial number
    virtual const std::string &getDependentSensor() const = 0;

    virtual ~Control() {}

    // Returns the assigned GPIO pin
    int getGpio() const { return gpio; }

protected:
    int gpio;               // Stores the GPIO pin for control
    int ON_Value;           // Number representing the ON value
    int OFF_Value;          // Number representing the OFF value
    std::string Dependent_Sensor;  // Serial number of the dependent sensor
};

#endif // CONTROL_H
