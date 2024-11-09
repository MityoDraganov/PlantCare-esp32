#include "MoistureSensor.h"
#include <algorithm>
#include <Arduino.h>


static MoistureSensor moistureSensor;

MoistureSensor::MoistureSensor() : readIndex(0), total(0), average(0) {
    Serial.println("MoistureSensor constructor called");
    SensorManager::registerSensor(this);
    std::fill(std::begin(readings), std::end(readings), 0);
}

void MoistureSensor::init(int gpioPin) {
    gpio = gpioPin;  
}

int MoistureSensor::readValue() {
    // Average multiple readings to smooth the value
    total = 0;
    for (int i = 0; i < NUM_READINGS; i++) {
        readings[i] = analogRead(gpio);
        total += readings[i];
    }
    average = total / NUM_READINGS;

    // Convert the averaged sensor reading to a percentage
    int percentage = (DRY_SOIL_VALUE - average) * 100 / (DRY_SOIL_VALUE - WET_SOIL_VALUE);

    // Clamp the percentage to be within 0 - 100 range
    percentage = std::max(0, std::min(100, percentage));

    return percentage; 
}



const char* MoistureSensor::getType() {
    return "MoistureSensor";  // Return the sensor type as a string
}
