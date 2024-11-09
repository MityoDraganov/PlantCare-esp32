#ifndef MOISTURESENSOR_H
#define MOISTURESENSOR_H

#include "../Sensor/Sensor.h"
#include "../SensorManager/SensorManager.h"
#include "driver/adc.h"

class MoistureSensor : public Sensor
{
public:
    MoistureSensor(); // Constructor declaration

    void init(int gpioPin) override; // Initialize the moisture sensor (ADC setup)
    int readValue() override;        // Read and return the moisture level as a percentage
    const char *getType() override;  // Return the type of the sensor

private:

    // Calibration values (optional, based on your sensor)
    static const int DRY_SOIL_VALUE = 4095;
    static const int WET_SOIL_VALUE = 1030;

    static const int NUM_READINGS = 10;

    int readings[NUM_READINGS]; // Array to store multiple readings for averaging
    int readIndex;              // Current index for reading
    long total;                 // Sum of all readings
    int average;                // Averaged reading
};

#endif // MOISTURESENSOR_H
