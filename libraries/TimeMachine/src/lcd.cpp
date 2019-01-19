#include "lcd.h"

LCD::LCD() : 
    module(0x27,20,4)
{
    for(int i=0; i<4; i++) {
        memset(logLines[i], 0, 21);
    }
}

void
LCD::begin()
{
    module.init();
    module.backlight();

    // module.setCursor ( 0, 0 );            // go to the top left corner
    // module.print("     GlamCocks      "); // write this string on the top row
    // module.setCursor ( 0, 1 );            // go to the third row
    // module.print("     Time Warp!     "); // pad with spaces for centering

    module.setCursor(0,3);
    module.print("GlamCocks");

    Log.addPrint(this);
}

void
LCD::loop()
{
    // uint32_t now = millis();

    // if (now - _lastPing > 1000) {
    //     //module.begin(20,4);
    //     Log.printf("Ping! %d", now);
    //     _lastPing = now;
    // }

    // if (_needsUpdate && now - _lastUpdate > 500) {
    //     displayLog();
    //     _needsUpdate = false;
    //     _lastUpdate = now;
    // }
}

void
LCD::setMode(LCDMode mode)
{
    this->mode = mode;
    fullUpdate();
}

void
LCD::setYear(uint32_t year)
{
    this->year = year;
    updateYear();
}


void
LCD::setMonth(uint8_t month)
{
    this->month = month;
    updateMonth();
}

void
LCD::setDay(uint8_t day)
{
    this->day = day;
    updateDay();
}

void
LCD::setSelVal(uint8_t val)
{
    this->selVal = val;
    updateSelVal();
}

void
LCD::setButton(Button which, bool set)
{
    switch(which) {
    case BtnYear: 
        module.setCursor(18,0);
        break;

    case BtnMonth: 
        module.setCursor(18,1);
        break;

    case BtnDay: 
        module.setCursor(18,2);
        break;

    case BtnSel: 
        module.setCursor(18,3);
        Log.printf("Sel set=%c\n", set ? 'T' : 'F');
        break;

    default:
        // Don't care about other buttons
        return;
    }

    char c = set ? '*' : ' ';
    module.write(c);
}


void
LCD::fullUpdate()
{
    module.clear();

    switch(mode) {
    case ModeDate:
        updateYear();
        updateMonth();
        updateDay();
        updateSelVal();
        break;

    case ModeLog:
        displayLog();
    }
}

void
LCD::updateYear()
{
    module.setCursor(0,0);
    module.printf("%5d", year);
}


void
LCD::updateMonth()
{
    module.setCursor(0,1);
    module.printf("%3d", month);
    
}

void
LCD::updateDay()
{
    module.setCursor(0,2);
    module.printf("%3d", day);
}

void
LCD::updateSelVal()
{
    module.setCursor(12,3);
    module.printf("%3d", selVal);
}

void
LCD::displayLog()
{
    // module.noDisplay();
    for(uint8_t i=0; i<4; i++) {
        module.setCursor(0, i);
        module.print(logLines[i]);
    }
    // module.display();
    // module.home();
}


///////////////////////
// Stream interface for logging

void
LCD::flush(void)
{
}

size_t
LCD::write(uint8_t c)
{
    // Should buffer these, but we will not for right now...
    return write(&c, 1);
}

size_t
LCD::write(const uint8_t *buffer, size_t size)
{
    if (!buffer || !size) return 0;

    // Move old values up
    for(uint8_t i=0; i<3; i++) {
        memcpy(logLines[i], logLines[i+1], 20);
    }
    // Erase log line 4
    memset(logLines[3],32,20);
    if (size>20) {
        size = 20;
    }
    strncpy(logLines[3], (const char *)buffer, size);

    logUpdated = true;

    return size;
}
