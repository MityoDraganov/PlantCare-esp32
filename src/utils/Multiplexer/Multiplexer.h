#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include <Arduino.h>

class Multiplexer {
public:
    Multiplexer(int selectAPin = 12, int selectBPin = 13);  // Default GPIO 12 and 13
    void begin();
    void selectChannel(uint8_t channel);
    uint8_t readChannel();
private:
    void _setSelectPins(uint8_t channel);
    int _selectAPin;
    int _selectBPin;
};

#endif
