#include "utils/Multiplexer/Multiplexer.h"

// Constructor: initializes the pins used for selection
Multiplexer::Multiplexer(int selectAPin, int selectBPin)
    : _selectAPin(selectAPin), _selectBPin(selectBPin) {}

// Begin method: sets up the GPIOs for the select pins
void Multiplexer::begin()
{
        pinMode(_selectAPin, OUTPUT);
        pinMode(_selectBPin, OUTPUT);
}

// Select a channel on the multiplexer (0-3)
void Multiplexer::selectChannel(uint8_t channel)
{
    // Check if the channel is within valid range (0-3)
    if (channel > 3)
    {
        Serial.println("Error: Channel out of range. Selecting channel 0 instead.");
        channel = 0; // Optionally handle this differently
    }
    _setSelectPins(channel);
}

// Helper function to set the appropriate values on the select pins
void Multiplexer::_setSelectPins(uint8_t channel)
{
    // Set voltage levels based on channel number
    switch (channel)
    {
        case 0: // Channel 0: L (0V) for both select A and B
            digitalWrite(_selectAPin, LOW);
            digitalWrite(_selectBPin, LOW);
            break;
        case 1: // Channel 1: L (0V) for select A, H (5V) for select B
            digitalWrite(_selectAPin, HIGH);
            digitalWrite(_selectBPin, LOW);
            break;
        case 2: // Channel 2: H (5V) for select A, L (0V) for select B
            digitalWrite(_selectAPin, LOW);
            digitalWrite(_selectBPin, HIGH);
            break;
        case 3: // Channel 3: H (5V) for both select A and B
            digitalWrite(_selectAPin, HIGH);
            digitalWrite(_selectBPin, HIGH);
            break;
    }
}

// Method to read the state of the select pins
uint8_t Multiplexer::readChannel()
{
    uint8_t channel = 0;
    channel |= digitalRead(_selectAPin);        // Read S0
    channel |= (digitalRead(_selectBPin) << 1); // Read S1
    return channel;                             // Returns the channel number (0-3)
}
