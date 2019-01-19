#pragma once

#include "lcd.h"
#include "encoder.h"

class TimeMachine {
public:
    TimeMachine();

    void begin();
    void loop();

private:
    LCD _lcd;
    Encoder _encYear;
};