#include "SoilTemperatureSensor.h"
#include <Arduino.h>
#include <cmath>

// Static instance of the TemperatureSensor
static TemperatureSensor temperatureSensor;

// Constants for thermistor and resistor
const float TemperatureSensor::R1 = 10000;  // 10KΩ resistor value
const float TemperatureSensor::BETA = 3435; // Beta coefficient for the thermistor
const float TemperatureSensor::T0 = 298.15; // Room temperature in Kelvin (25°C = 298.15K)
const float TemperatureSensor::R0 = 10000;  // Resistance of the thermistor at 25°C (10KΩ)

TemperatureSensor::TemperatureSensor() : readIndex(0), total(0), average(0)
{
    Serial.println("TemperatureSensor constructor called");
    SensorManager::registerSensor(this);                    // Register with the SensorManager
    std::fill(std::begin(readings), std::end(readings), 0); // Initialize readings array
}

void TemperatureSensor::init(int gpioPin)
{
    gpio = gpioPin;
}
int TemperatureSensor::readValue()
{
    total = 0;
    for (int i = 0; i < NUM_READINGS; i++)
    {
        readings[i] = analogRead(gpio);
        total += readings[i];
    }
    average = total / NUM_READINGS;

    float temperatureC = calculateTemperature(average);

    // Convert the temperature to an integer value (Celsius)
    return static_cast<int>(temperatureC);
}

float TemperatureSensor::calculateTemperature(int adcValue)
{
    // Convert the ADC value to a voltage (assuming 12-bit ADC, 3.3V reference)
    float voltage = adcValue * (3.3 / 4095.0);

    // Calculate the resistance of the thermistor
    float resistance = (3.3 / voltage - 1) * R1;

    // Calculate the temperature in Kelvin using the Beta equation
    float temperatureK = 1 / (1 / T0 + log(resistance / R0) / BETA);

    // Convert Kelvin to Celsius
    return temperatureK - 273.15;
}

const char *TemperatureSensor::getType()
{
    return "TemperatureSensor"; // Return the sensor type as a string
}
