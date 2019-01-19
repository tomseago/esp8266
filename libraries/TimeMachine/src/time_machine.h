#pragma once

#include "lcd.h"
#include "encoder.h"
#include <Bounce2.h>
#include "circles.h"
#include "photo_lights.h"

class TimeMachine {
public:
    TimeMachine();

    void begin();
    void loop();

    uint32_t targetYear = 2019;
    uint8_t targetMonth = 1;
    uint8_t targetDay = 19;

    int8_t selVal = 0;


    void goBtnLight(bool on);

private:
    // Hardware
    LCD lcd;

    Encoder encYear;
    Encoder encMonth;
    Encoder encDay;

    Encoder encSel;

    Bounce goBtn;

    Circles circles;
    PhotoLights photoLights;

    void readEncoders();

};