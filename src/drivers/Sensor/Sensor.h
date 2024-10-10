#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
public:
    virtual void init() = 0;         // To initialize the sensor
    virtual int readValue() = 0;     // To read the sensor value
    virtual const char* getType() = 0; // To get the sensor type
    virtual ~Sensor() {}
};

#endif // SENSOR_H