// #include "moistureSensor.h"
// #include <Arduino.h>

// // Define the pin where the moisture sensor is connected
// const int MOISTURE_SENSOR_PIN = A0; // ADC1_0 A0 == ADC1_0 == VP

// void initMoistureSensor()
// {
//     pinMode(MOISTURE_SENSOR_PIN, INPUT);
// }

// int readMoistureLevel()
// {
//     int sensorValue = analogRead(MOISTURE_SENSOR_PIN);
//     // Assuming the sensor returns higher values for higher moisture
//     // int invertedValue = 4095 - sensorValue; // Invert the reading if higher value means more moisture
//     return sensorValue;
// }

#include "moistureSensor.h"
#include <Arduino.h>

// Define the pin where the moisture sensor is connected
const int MOISTURE_SENSOR_PIN = A0; // ADC1_0 A0 == ADC1_0 == VP

// Calibration values based on your measurements
const int DRY_SOIL_VALUE = 4095; // Completely dry soil
const int WET_SOIL_VALUE = 1030; // Most moist condition

// Number of readings to average for smoothing
const int NUM_READINGS = 10;

// Array to hold the sensor readings for averaging
int readings[NUM_READINGS];
int readIndex = 0;
long total = 0;
int average = 0;

void initMoistureSensor() {
    pinMode(MOISTURE_SENSOR_PIN, INPUT);

    // Initialize all readings to the initial sensor value
    for (int i = 0; i < NUM_READINGS; i++) {
        readings[i] = analogRead(MOISTURE_SENSOR_PIN);
        total += readings[i];
    }
}

int readMoistureLevel() {
    // Subtract the last reading
    total -= readings[readIndex];
    
    // Read the sensor
    readings[readIndex] = analogRead(MOISTURE_SENSOR_PIN);
    
    // Add the new reading to the total
    total += readings[readIndex];
    
    // Advance to the next position in the array
    readIndex = (readIndex + 1) % NUM_READINGS;
    
    // Calculate the average of the readings
    average = total / NUM_READINGS;

    // Clamp the average value to within the expected range
    if (average > DRY_SOIL_VALUE) {
        average = DRY_SOIL_VALUE;
    } else if (average < WET_SOIL_VALUE) {
        average = WET_SOIL_VALUE;
    }

    // Normalize the average sensor value to a percentage (0% dry, 100% wet)
    int moisturePercentage = map(average, DRY_SOIL_VALUE, WET_SOIL_VALUE, 0, 100);

    return moisturePercentage;
}