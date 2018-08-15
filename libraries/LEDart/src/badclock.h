#pragma once

#include "LEDArt.h"
#include "buttonwatcher.h"
// #include "nexus.h"

class BadClock : public ButtonWatcher {
public:
    class DigitAnim : public LEDArtAnimation {
    public:
        DigitAnim(BadClock& parent);

        virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);

    private:
        BadClock& parent;
    };

    DigitAnim digitAnim;

    uint8_t digits[4];
    bool showDots;

    BadClock(Nexus& nexus, LEDArtPiece& piece);

    virtual void begin();
    virtual void loop();

    virtual void buttonFell(uint8_t button);
    virtual void buttonLongPress(uint8_t button);

private:
    Nexus& nexus;
    LEDArtPiece& piece;

    bool inSetMode;
    bool showSeconds;
    uint32_t offset;

    void resetSeconds();
};
