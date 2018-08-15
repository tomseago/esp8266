#pragma once

#include "LEDArt.h"

#include <Bounce2.h>

class ButtonWatcher {
public:
    Bounce buttons[4];
    uint32_t downAt[4];
    bool notified[4];
    uint16_t longThreshold;

    ButtonWatcher(uint16_t longThreshold=4000);

    virtual void begin();
    virtual void loop();

    virtual void buttonRose(uint8_t button) {};
    virtual void buttonFell(uint8_t button) {};
    virtual void buttonLongPress(uint8_t button) {};
};
