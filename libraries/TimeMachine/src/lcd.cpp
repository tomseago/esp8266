#include "lcd.h"

LCD::LCD() : 
    _module(0x27,20,4)
{
    for(int i=0; i<4; i++) {
        memset(_logLines[i], 0, 21);
    }
}

void
LCD::begin()
{
    _module.init();
    _module.backlight();

    _module.setCursor ( 0, 0 );            // go to the top left corner
    _module.print("     GlamCocks      "); // write this string on the top row
    _module.setCursor ( 0, 1 );            // go to the third row
    _module.print("     Time Warp!     "); // pad with spaces for centering

    Log.addPrint(this);
}

void
LCD::loop()
{
    uint32_t now = millis();

    if (now - _lastPing > 1000) {
        //_module.begin(20,4);
        Log.printf("Ping! %d", now);
        _lastPing = now;
    }

    if (_needsUpdate && now - _lastUpdate > 500) {
        displayLog();
        _needsUpdate = false;
        _lastUpdate = now;
    }
}

void
LCD::displayLog()
{
    // _module.noDisplay();
    for(uint8_t i=0; i<4; i++) {
        _module.setCursor(0, i);
        _module.print(_logLines[i]);
    }
    // _module.display();
    // _module.home();
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
        memcpy(_logLines[i], _logLines[i+1], 20);
    }
    // Erase log line 4
    memset(_logLines[3],32,20);
    if (size>20) {
        size = 20;
    }
    strncpy(_logLines[3], (const char *)buffer, size);

    _needsUpdate = true;
}
