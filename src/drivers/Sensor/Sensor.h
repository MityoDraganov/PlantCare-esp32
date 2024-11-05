#ifndef SENSOR_H
#define SENSOR_H

class Sensor
{
public:
    virtual void init(int gpioPin) = 0; // To initialize the sensor
    virtual int readValue() = 0;        // To read the sensor value
    virtual const char *getType() = 0;  // To get the sensor type
    virtual ~Sensor() {}
    int getGpio() const { return gpio; }

protected:    // Make GPIO pin protected
    int gpio; // Store the GPIO pin for ADC reading
};

#endif // SENSOR_H