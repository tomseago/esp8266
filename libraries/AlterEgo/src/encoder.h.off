#pragma once

#include <Esp.h>

class Encoder {
public:
    Encoder(uint8_t clockPin, uint8_t dataPin, uint8_t swPin);

    void begin();
    void loop();

    void clockChange();
    void dataChange();
    void swChange();

private:
    uint8_t _clockPin;
    uint8_t _dataPin;
    uint8_t _swPin;

    uint8_t _changeMask = 0;
    uint8_t _changeCount = 0;
    int8_t _delta = 0;

    bool _readyForData;
    int32_t _val = 0;
};