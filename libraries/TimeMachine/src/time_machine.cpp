#include "time_machine.h"

#define GO_BTN_PIN 23
#define GO_BTN_LIGHT_PIN 18

TimeMachine::TimeMachine() :
    encDay(36,39,32),
    encMonth(33,34,35),
    encYear(25,26,27),
    encSel(14,12,13)
{

}

void
TimeMachine::begin()
{
    encYear.begin();
    encMonth.begin();
    encDay.begin();
    encSel.begin();

    goBtn.attach(GO_BTN_PIN, INPUT_PULLUP);

    lcd.begin();
    lcd.setYear(targetYear);
    lcd.setMonth(targetMonth);
    lcd.setDay(targetDay);
    lcd.setSelVal(selVal);

    pinMode(GO_BTN_LIGHT_PIN, OUTPUT);


    pinMode(0, OUTPUT);
    pinMode(4, OUTPUT);

    circles.begin();
    photoLights.begin();
}

void
TimeMachine::loop()
{
    readEncoders();

    goBtn.update();
    goBtnLight(!goBtn.read());
    //goBtnLight(true);

    lcd.loop();
    circles.loop();
    photoLights.loop();
}

void
TimeMachine::readEncoders()
{
    int16_t d;

    d = encYear.takeDelta();
    if (d!=0) {
        targetYear += d;
        lcd.setYear(targetYear);
    }
    if (encYear.swUpdate()) {
        lcd.setButton(LCD::BtnYear, encYear.swPressed());
    }

    d = encMonth.takeDelta();
    if (d!=0) {
        targetMonth += d;
        lcd.setMonth(targetMonth);
    }
    if (encMonth.swUpdate()) {
        lcd.setButton(LCD::BtnMonth, encMonth.swPressed());
    }

    d = encDay.takeDelta();
    if (d!=0) {
        targetDay += d;
        lcd.setDay(targetDay);
    }
    if (encDay.swUpdate()) {
        lcd.setButton(LCD::BtnDay, encDay.swPressed());
    }

    d = encSel.takeDelta();
    if (d!=0) {
        selVal += d;
        lcd.setSelVal(selVal);
    }
    if (encSel.swUpdate()) {
        lcd.setButton(LCD::BtnSel, encSel.swPressed());
    }


    //// Hacky debugging a pin thing
    if (selVal > 0) {
        // On
        digitalWrite(0, HIGH);
        digitalWrite(4, HIGH);
    } else {
        // Off
        digitalWrite(0, LOW);
        digitalWrite(4, LOW);
    }
}

void
TimeMachine::goBtnLight(bool on)
{
    digitalWrite(GO_BTN_LIGHT_PIN, on);
}