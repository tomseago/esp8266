#include "encoder.h"

#include <log.h>

#define CLOCK_BIT (1 << 2)
#define DATA_BIT  (1 << 1)
#define SW_BIT    (1)

void
isrClockChange(void* arg)
{
    ((Encoder*)arg)->clockChange();
}

void
isrDataChange(void* arg)
{
    ((Encoder*)arg)->dataChange();
}

void
isrSwChange(void* arg)
{
    ((Encoder*)arg)->swChange();
}

Encoder::Encoder(uint8_t clockPin, uint8_t dataPin, uint8_t swPin) :
    _clockPin(clockPin),
    _dataPin(dataPin),
    _swPin(swPin)
{

}

void
Encoder::begin()
{
    pinMode(_clockPin, INPUT_PULLUP);
    pinMode(_dataPin, INPUT_PULLUP);
    pinMode(_swPin, INPUT_PULLUP);

    attachInterruptArg(_clockPin, isrClockChange, this, CHANGE);
    // attachInterruptArg(_dataPin, isrDataChange, this, CHANGE);
    attachInterruptArg(_swPin, isrSwChange, this, CHANGE);
}

void
Encoder::loop()
{
    uint8_t cm = _changeMask;
    int8_t d = _delta;
    _changeMask = 0;
    _delta = 0;

    // bool clk = digitalRead(_clockPin);
    // bool data = digitalRead(_dataPin);

    // if (clk == _readyForData) {
    //     return;
    // }

    // _readyForData = clk;
    // bool cm = false;
    // int8_t d = 0;

    // if (clk) {
    //     d = data ? 1 : -1;
    // } else {
    //     d = data ? -1 : 1;
    // }

    if (cm)
    {
        // uint8_t pins = (digitalRead(_clockPin) << 2) + (digitalRead(_dataPin) << 1) + (digitalRead(_swPin));
        // Log.printf("C: %c%c%c,%c%c%c %d %d ", 
        //     cm&4 ? 'C' : '.', cm&2 ? 'D' : '.', cm&1 ? 'S' : '.',
        //     pins&4 ? 'C' : '.', pins&2 ? 'D' : '.', pins&1 ? 'S' : '.',
        //     _changeCount, d);
        if (d > 0) {
            _val++;
        } else {
            _val--;
        }
        Log.printf("Value = %d, %d              ", d, _val);
    }
}

void
Encoder::clockChange()
{
    if (digitalRead(_clockPin)) {
        _delta = digitalRead(_dataPin) ? 1 : -1;
    } else {
        _delta = digitalRead(_dataPin) ? -1: 1;        
    }
    _changeMask |= CLOCK_BIT;
    _changeCount++;

    // if (_readyForData) {
    //     // Ignore this interrupt
    //     return;
    // }

    // _readyForData = true;

    // // uint8_t pins = (digitalRead(_clockPin) << 1) + digitalRead(_dataPin);

    // // detachInterrupt(_clockPin);
    // // attachInterruptArg(_dataPin, isrDataChange, this, CHANGE);    

    // // _changeMask |= CLOCK_BIT;
    // // _changeCount++;

    // // if (digitalRead(_dataPin)) {
    // //     _delta++;
    // // } else {
    // //     _delta--;
    // // }
}

void
Encoder::dataChange()
{
    if (!_readyForData) {
        // Ignore 
        return;
    }

    _readyForData = false;

    if (digitalRead(_dataPin)) {
        _delta++;
    } else {
        _delta--;
    }
    _changeMask |= DATA_BIT;
    _changeCount++;
}

void
Encoder::swChange()
{
    _changeMask |= SW_BIT;
    _changeCount++;
}