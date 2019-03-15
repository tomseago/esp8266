#include "alter_ego.h"

#define GO_BTN_PIN 23
#define GO_BTN_LIGHT_PIN 18

#define PHOTO_LIGHTS_PIN 2
#define CIRCLES_PIN 15

AlterEgo::AlterEgo() :
    encRight(36,39,32),
    encMiddle(33,34,35),
    encLeft(25,26,27),
    encSel(14,19,13),
    leftWall(PHOTO_LIGHTS_PIN),
    rightWall(CIRCLES_PIN)
{

}

void
AlterEgo::begin()
{
    encLeft.begin();
    encMiddle.begin();
    encRight.begin();
    encSel.begin();

    goBtn.attach(GO_BTN_PIN, INPUT_PULLUP);

    lcd.begin();
    lcd.setLeft(leftVal);
    lcd.setRight(rightVal);
    lcd.setSat(satVal);
    lcd.setBright(brightVal);

    pinMode(GO_BTN_LIGHT_PIN, OUTPUT);


    pinMode(0, OUTPUT);
    pinMode(4, OUTPUT);

    leftWall.begin();
    rightWall.begin();

    // leftWall.setHue(0.5);
    // rightWall.setSat(0.5);
    resetAll();
}

void
AlterEgo::loop()
{
    readEncoders();

    goBtn.update();
    goBtnLight(!goBtn.read());
    if (goBtn.fell()) {
        //resetAll();
        swapSides();
    }
    //goBtnLight(true);

    lcd.loop();
    leftWall.loop();
    rightWall.loop();
}

void
AlterEgo::resetAll() 
{
    setLeft(0);
    setRight( MAX_ENC_VAL / 2);
    setSat(MAX_ENC_VAL);
    setBright(MAX_ENC_VAL);
}

void
AlterEgo::swapSides()
{
    int32_t tmp = leftVal;
    setLeft(rightVal);
    setRight(tmp);
}

int32_t updateVal(int32_t old, int16_t delta, bool rollover) {
    int32_t out = old + delta;
    if (out < 0) {
        if (rollover) {
            out += MAX_ENC_VAL;
        } else {
            out = 0;
        }
    }
    if (out > MAX_ENC_VAL) {
        if (rollover) {
            out -= MAX_ENC_VAL;
        } else {
            out = MAX_ENC_VAL;
        }
    }
    return out;
}

float valToFloat(uint32_t val) {
    return (float) val / (float)MAX_ENC_VAL;
}

void
AlterEgo::setLeft(int32_t val) {
    leftVal = val;
    leftWall.setHue(valToFloat(leftVal));
    lcd.setLeft(leftVal);
}

void
AlterEgo::setRight(int32_t val) {
    rightVal = val;
    rightWall.setHue(valToFloat(rightVal));
    lcd.setRight(rightVal);
}

void
AlterEgo::setSat(int32_t val) {
    satVal = val;
    float v = valToFloat(satVal);
    leftWall.setSat(v);
    rightWall.setSat(v);
    lcd.setSat(satVal);
}

void
AlterEgo::setBright(int32_t val) {
    brightVal = val;
    float v = valToFloat(brightVal);
    leftWall.setBright(v);
    rightWall.setBright(v);
    lcd.setBright(brightVal);
}

void
AlterEgo::readEncoders()
{
    int16_t d;

    d = encLeft.takeDelta();
    if (d!=0) {
        setLeft(updateVal(leftVal, d, true));
   }

    d = encRight.takeDelta();
    if (d!=0) {
        setRight(updateVal(rightVal, d, true));
    }

    d = encMiddle.takeDelta();
    if (d!=0) {
        setSat(updateVal(satVal, d, false));
    }
    if (encMiddle.swUpdate()) {
        if (!encMiddle.swPressed()) {
            setSat(MAX_ENC_VAL);
        }
    }

    d = encSel.takeDelta();
    if (d!=0) {
        setBright(updateVal(brightVal, d, false));
    }

    // d = encYear.takeDelta();
    // if (d!=0) {
    //     targetYear += d;
    //     lcd.setYear(targetYear);
    // }
    // if (encYear.swUpdate()) {
    //     lcd.setButton(LCD::BtnYear, encYear.swPressed());
    // }

    // d = encMonth.takeDelta();
    // if (d!=0) {
    //     targetMonth += d;
    //     lcd.setMonth(targetMonth);
    // }
    // if (encMonth.swUpdate()) {
    //     lcd.setButton(LCD::BtnMonth, encMonth.swPressed());
    // }

    // d = encDay.takeDelta();
    // if (d!=0) {
    //     targetDay += d;
    //     lcd.setDay(targetDay);
    // }
    // if (encDay.swUpdate()) {
    //     lcd.setButton(LCD::BtnDay, encDay.swPressed());
    // }

    // d = encSel.takeDelta();
    // if (d!=0) {
    //     selVal += d;
    //     lcd.setSelVal(selVal);
    // }
    // if (encSel.swUpdate()) {
    //     lcd.setButton(LCD::BtnSel, encSel.swPressed());
    // }


    //// Hacky debugging a pin thing
    // if (selVal > 0) {
    //     // On
    //     digitalWrite(0, HIGH);
    //     digitalWrite(4, HIGH);
    // } else {
    //     // Off
    //     digitalWrite(0, LOW);
    //     digitalWrite(4, LOW);
    // }
}

void
AlterEgo::goBtnLight(bool on)
{
    digitalWrite(GO_BTN_LIGHT_PIN, on);
}