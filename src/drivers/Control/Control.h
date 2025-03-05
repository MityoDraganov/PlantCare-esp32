#ifndef CONTROL_H
#define CONTROL_H

#include <string>

class Control
{
public:

    // Method to start the control
    virtual void start() = 0;

    // Method to stop the control
    virtual void stop() = 0;

    virtual void init(int gpioPin) = 0;

    // To get the ON value for the control
    virtual int getMinValue() const = 0;

    // To get the OFF value for the control
    virtual int getMaxValue() const = 0;

    // Method to set the ON value for the control
    virtual void setMinValue(int value) = 0;

    // Method to set the OFF value for the control
    virtual void setMaxValue(int value) = 0;

    // To get the dependent sensor's serial number
    virtual const std::string &getDependentSensor() const = 0;

    // To get the type of the control
    virtual const std::string& getType() const = 0;

    virtual ~Control() {}

    // Returns the assigned GPIO pin
    int getGpio() const { return gpio; }

protected:
    int gpio;               // Stores the GPIO pin for control
    int Min_Value;           // Number representing the ON value
    int Max_Value;          // Number representing the OFF value
    std::string Dependent_Sensor;  // Serial number of the dependent sensor
};

#endif // CONTROL_H
