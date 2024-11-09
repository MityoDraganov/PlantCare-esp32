#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include "../Sensor/Sensor.h"
#include "../SensorManager/SensorManager.h"
#include "driver/adc.h"
#include <algorithm>
#include <iterator>

class TemperatureSensor : public Sensor
{
public:
    TemperatureSensor(); // Constructor declaration

    void init(int gpioPin) override;
    int readValue() override;
    const char *getType() override;

private:
    // Constants for the thermistor and resistor
    static const int NUM_READINGS = 10; // Number of readings for smoothing
    static const float R1;              // Fixed resistor value (10KΩ)
    static const float BETA;            // Beta coefficient for the thermistor
    static const float T0;              // Room temperature in Kelvin (25°C = 298.15K)
    static const float R0;              // Resistance of the thermistor at 25°C (10KΩ)

    int readings[NUM_READINGS]; // Array to store multiple readings for averaging
    int readIndex;              // Current index for reading
    long total;                 // Sum of all readings
    int average;                // Averaged reading

    int readADC();                            // Function to read the raw ADC value
    float calculateTemperature(int adcValue); // Function to calculate temperature from ADC value
};

#endif // TEMPERATURESENSOR_H
