#include "encoder.h"

// #include <log.h>

// #define CLOCK_BIT (1 << 2)
// #define DATA_BIT  (1 << 1)
// #define SW_BIT    (1)

#define DEBOUNCED_STATE 0
#define UNSTABLE_STATE  1
#define STATE_CHANGED   3


void
isrUpdateRotationState(void* arg)
{
    ((Encoder*)arg)->updateRotationState();
}

// void
// isrSwChange(void* arg)
// {
//     ((Encoder*)arg)->swChange();
// }

Encoder::Encoder(uint8_t clockPin, uint8_t dataPin, uint8_t swPin) :
    clockPin(clockPin),
    dataPin(dataPin),
    swPin(swPin)
{

}

void
Encoder::begin()
{
    pinMode(clockPin, INPUT);
    pinMode(dataPin, INPUT);
    pinMode(swPin, INPUT);

    attachInterruptArg(clockPin, isrUpdateRotationState, this, CHANGE);
    attachInterruptArg(dataPin, isrUpdateRotationState, this, CHANGE);
    //attachInterruptArg(swPin, isrSwChange, this, CHANGE);

    if (digitalRead(swPin)) {
        swState = _BV(DEBOUNCED_STATE) | _BV(UNSTABLE_STATE);
    }

    swPrevMillis = millis();
}

int16_t
Encoder::takeDelta()
{
    int16_t ret = delta;
    delta = 0;
    return ret;
}

#define BOUNCE_INTERVAL 15

bool
Encoder::swUpdate()
{
    // Read the state of the switch in a temporary variable.
    bool currentState = digitalRead(swPin);
    swState &= ~_BV(STATE_CHANGED);

    // If the reading is different from last reading, reset the debounce counter
    if ( currentState != (bool)(swState & _BV(UNSTABLE_STATE)) ) {
        swPrevMillis = millis();
        swState ^= _BV(UNSTABLE_STATE);
    } else
        if ( millis() - swPrevMillis >= BOUNCE_INTERVAL ) {
            // We have passed the threshold time, so the input is now stable
            // If it is different from last state, set the STATE_CHANGED flag
            if ((bool)(swState & _BV(DEBOUNCED_STATE)) != currentState) {
                swPrevMillis = millis();
                swState ^= _BV(DEBOUNCED_STATE);
                swState |= _BV(STATE_CHANGED);
            }
        }

    return swState & _BV(STATE_CHANGED);
}

bool
Encoder::swPressed()
{
    return !(swState & _BV(DEBOUNCED_STATE));
}


// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Anti-clockwise step.
#define DIR_CCW 0x20


/*
 * The below state table has, for each state (row), the new state
 * to set based on the next encoder output. From left to right in,
 * the table, the encoder outputs are 00, 01, 10, 11, and the value
 * in that position is the new state to set.
 */

#define R_START 0x0

#ifdef HALF_STEP
// Use the half-step state table (emits a code at 00 and 11)
#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5
const unsigned char ttable[6][4] = {
  // R_START (00)
  {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
  // R_CCW_BEGIN
  {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
  // R_CW_BEGIN
  {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
  // R_START_M (11)
  {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
  // R_CW_BEGIN_M
  {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
  // R_CCW_BEGIN_M
  {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
};
#else
// Use the full-step state table (emits a code at 00 only)
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};
#endif

void
Encoder::updateRotationState()
{
  // Grab state of input pins.
  unsigned char pinstate = (digitalRead(dataPin) << 1) | digitalRead(clockPin);

  // Determine new state from the pins and state table.
  rotState = ttable[rotState & 0xf][pinstate];

  // Return emit bits, ie the generated event.
  uint8_t result = rotState & 0x30;

  if (result & DIR_CW) {
    delta += 1;
  } else if (result & DIR_CCW) {
    delta -= 1;
  }
}