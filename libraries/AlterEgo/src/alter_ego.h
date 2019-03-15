#pragma once

#include "lcd.h"
#include "encoder.h"
#include <Bounce2.h>
#include "photo_lights.h"

#define MAX_ENC_VAL 100

class AlterEgo {
public:
    AlterEgo();

    void begin();
    void loop();

    int32_t leftVal = 0;
    int32_t rightVal = MAX_ENC_VAL / 2;

    int32_t satVal = MAX_ENC_VAL;
    int32_t brightVal = MAX_ENC_VAL;


    void goBtnLight(bool on);

private:
    // Hardware
    LCD lcd;

    Encoder encLeft;
    Encoder encMiddle;
    Encoder encRight;

    Encoder encSel;

    Bounce goBtn;

    PhotoLights<NeoEsp32I2s0800KbpsMethod> leftWall;
    PhotoLights<NeoEsp32I2s1800KbpsMethod> rightWall;
    // PhotoLights rightWall;

    void resetAll();
    void swapSides();
    
    void setLeft(int32_t val);
    void setRight(int32_t val);
    void setSat(int32_t val);
    void setBright(int32_t val);

    void readEncoders();

};