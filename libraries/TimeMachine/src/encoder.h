#pragma once

#include <Esp.h>

#include "debug_buffer.h"

class Encoder {
public:
    Encoder(uint8_t clockPin, uint8_t dataPin, uint8_t swPin);

    void begin();

    int16_t takeDelta();

    bool swUpdate(); // Returns true if the state changed
    bool swPressed();     // Returns the current state of the switch

    // isrs
    void updateRotationState();
    // void swChange();

private:
    uint8_t clockPin;
    uint8_t dataPin;
    uint8_t swPin;

    int16_t delta = 0;

    // Rotation state
    uint8_t rotState;

    // Switch state
    uint32_t swPrevMillis;
    uint8_t swState;
};