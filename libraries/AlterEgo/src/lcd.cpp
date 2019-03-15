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

    module.setCursor(6,0);
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
LCD::setLeft(int32_t v)
{
    this->left = v;
    updateLeft();
}

void
LCD::setRight(int32_t v)
{
    this->right = v;
    updateRight();
}

void
LCD::setSat(int32_t v)
{
    this->sat = v;
    updateSat();
}

void
LCD::setBright(int32_t v)
{
    this->bright = v;
    updateBright();
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
        updateLeft();
        updateRight();
        updateSat();
        updateBright();
        break;

    case ModeLog:
        displayLog();
    }
}

void
LCD::updateLeft()
{
    module.setCursor(0,1);
    module.printf("%3d", left);
}


void
LCD::updateRight()
{
    module.setCursor(17,1);
    module.printf("%3d", right);
    
}

void
LCD::updateSat()
{
    module.setCursor(9,2);
    module.printf("%3d", sat);
    
}

void
LCD::updateBright()
{
    module.setCursor(9,3);
    module.printf("%3d", bright);
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
