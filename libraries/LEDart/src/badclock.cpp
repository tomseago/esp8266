#include "badclock.h"


/*
    TODOS:

    - Add 12 hour time mode
    - Add seconds mode
    - Allow other animations to set pixel colors, this will require geometry
    - Add skew factor
    - Make skew factor settable
    - Add time and skew to web UI
    - Add character font
    - Add set mode messages for like 1 second
    - Add messages    
*/

// #include <FS.h>
// #include <string.h>

#include "log.h"
// #include "butil.h"

// #include <StandardCplusplus.h>
// #include <string>

// using namespace std;

#include "math.h"

const LAColor red(255,0,0);
const LAColor yellow(255,255,0);
const LAColor green(0,255,0);
const LAColor cyan(0,255,255);
const LAColor blue(0,0,255);
const LAColor purple(255,0,255);

const LAColor white(255,255,255);
const LAColor black(0,0,0);

uint8_t digitFont[10] = { 0x77, 0x44, 0x3e, 0x6e, 0x4d, 0x6b, 0x7b, 0x46, 0x7f, 0x6f };

void
setSegment(LEDArtPiece& piece, uint8_t digit, uint8_t segment, LAColor color)
{
    uint8_t start = (28 * digit) + (4 * segment);

    for(uint8_t px=start; px < start + 4; px++) 
    {
        piece.strip.SetPixelColor(px, color);
    }
}

void
hoursAndMinutes(uint32_t seconds, uint8_t& hours, uint8_t& mins)
{
    hours = seconds / 3600;
    mins = (seconds / 60) - (hours * 60);

    // hours = hours % 24;
    hours = hours % 12;
    if (!hours) 
    {
        hours = 12;
    }
}

void
minutesAndSeconds(uint32_t seconds, uint8_t& mins, uint8_t& secInMin)
{
    uint8_t hours = seconds / 3600;
    mins = (seconds / 60) - (hours * 60);
    secInMin = seconds % 60;
}



BadClock::DigitAnim::DigitAnim(BadClock& parent) : 
    LEDArtAnimation("DigitAnim"),
    parent(parent)
{
    type = LEDAnimationType_STATUS;
    maxDuration = 0;
}

void 
BadClock::DigitAnim::animate(LEDArtPiece& piece, LEDAnimationParam p)
{
    // Turn everything on (in future expect other animations to do this)
    piece.strip.ClearTo(red);

    // Now go digit by digit and turn things off as necessary
    for(uint8_t i=0; i<4; i++) 
    {
        uint8_t d = parent.digits[i] % 10;

        uint8_t font = digitFont[d];

        // Special case no leading 0 (in 24 hour mode would we want it?)
        if (i==0 && d == 0)
        {
            font = 0;
        }

        for(uint8_t j=0; j<7; j++) 
        {
            if (! ((font >> j) & 0x01 ) ) {
                // Turn it off!
                setSegment(piece, i, j, black);
            }
        }
    }

    // Dots
    LAColor dotColor = parent.showDots ? (parent.inSetMode ? green : red ) : black;
    if (parent.inDebug)
    {
        dotColor = parent.ignoreVariance ? yellow : purple;
    }
    piece.strip.SetPixelColor(112, dotColor);
    piece.strip.SetPixelColor(113, dotColor);
}


BadClock::BadClock(Nexus& nexus, LEDArtPiece& piece) :
    ButtonWatcher(),
    digitAnim(*this), nexus(nexus), piece(piece)
{
    digits[0] = 0;
    digits[1] = 1;
    digits[2] = 2;
    digits[3] = 3;

    showDots = true;
}

void
BadClock::begin()
{
    ButtonWatcher::begin();
}

uint32_t lastDebugPrint = 0;

void
BadClock::loop()
{
    ButtonWatcher::loop();

    
    uint32_t secs = (millis() / 1000) + offset + debugOffset;

    // now vary it
    float vector = 0.0;
    float days = (float)secs / (3600.0 * 24.0);
    float maxVar = 0.0;
    int32_t variance = 0;
    if (!ignoreVariance)
    {
        // Period of about 37 minutes for up down vector
        vector = sinf(((secs % 2222) / 2222.0) * M_TWOPI);

        // days * one hour (at 7 days, +-7 hours)
        //maxVar = (secs / (24.0 * 3600.0) ) * 3600.0;
        //maxVar = secs / 24.0; // linear
        maxVar = powf((days / 5.0), 4) * 3600.0;

        // Magnitude
        variance = (int32_t)(vector * maxVar);
    }

    if (millis() - lastDebugPrint > 1000) 
    {
        lastDebugPrint = millis();
        Log.printf("secs=%d, vector=%f, days=%f, maxVar=%f, variance=%d\n", secs, vector, days, maxVar, variance);
    }

    secs += variance;

    /*
    digits[0] = secs / 1000;
    digits[1] = (secs / 100) % 10;
    digits[2] = (secs / 10) % 10;
    digits[3] = secs % 10;

    showDots = secs % 2;    
    digits[0] = digits[2];
    digits[1] = digits[3];
    // digits[2] = digits[3];
    */

    // Real time
    uint8_t hours,mins;

    if (showSeconds)
    {
        // Really want seconds...
        minutesAndSeconds(secs, hours, mins);
    }
    else
    {
        hoursAndMinutes(secs, hours, mins);
    }

    digits[0] = hours / 10;
    digits[1] = hours % 10;
    digits[2] = mins / 10;
    digits[3] = mins % 10;

    if (showSeconds)
    {
        showDots = true;
    }
    else if (inSetMode)
    {
        showDots = (millis() % 1000) > 500;    
    }
    else
    {
        showDots = secs % 2;
    }
}

void 
BadClock::buttonFell(uint8_t button) 
{
    if (inSetMode)
    {
        uint32_t secs = (millis() / 1000) + offset;

        uint8_t cHours,cMins, nHours, nMins;
        hoursAndMinutes(secs, cHours, cMins);

        switch(button)
        {
        case 2:
            if (showSeconds)
            {
                resetSeconds();
            }
            else
            {
                hoursAndMinutes(secs + 3600, nHours, nMins);
                if (nHours < cHours)
                {
                    offset -= 3600 * 23;
                }
                else
                {
                    offset += 3600;
                }
            }
            break;

        case 3:
            hoursAndMinutes(secs + 60, nHours, nMins);
            if (nHours != cHours)
            {
                offset -= 60 * 59;
            }
            else
            {
                offset += 60;
            }
            break;
        }
    }
    else if (inDebug)
    {
        switch(button)
        {
        case 0:
            // Add 10 minutes
            debugOffset += 600;
            break;

        case 1:
            // Add 1 hour
            debugOffset += 3600;
            break;

        case 2:
            debugOffset = 0;
            break;

        case 3:
            ignoreVariance = !ignoreVariance;
            break;
        }
    }
    else
    {
        // Not in set mode
    }
}


void 
BadClock::buttonLongPress(uint8_t button) 
{
    switch(button)
    {
    case 0:
        inSetMode = !inSetMode;
        break;

    case 1:
        showSeconds = !showSeconds;
        break;

    case 3:
        inDebug = !inDebug;
        break;
    }
}

void
BadClock::resetSeconds()
{
    uint32_t nowSecs = millis() / 1000;
    uint32_t secs = nowSecs + offset;

    uint8_t cHours, cMins;
    hoursAndMinutes(secs, cHours, cMins);

    // Now calculate an offset which will produce exactly the hours and mins
    // result.
    uint32_t zeroed = (cHours * 3600) + (cMins * 60);

    int32_t off = zeroed - nowSecs;
    if (off < 0)
    {
        off += 24 * 3600;
    }
    offset = off;
}

