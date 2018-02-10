#include "log.h"
#include <HardwareSerial.h>

// The maximum number of output streams which may be added. Serial
// can be turned on with a switch.
#define MAX_STREAMS 2

bool serialEnabled;
Stream* streams[MAX_STREAMS];

DBLog::DBLog()
{
    for(uint8_t i=0; i<MAX_STREAMS; i++) 
    {
        streams[i] = 0;
    }
    serialEnabled = false;
}

void DBLog::setSerialEnabled(bool en)
{
    serialEnabled = en;
}

bool
DBLog::addStream(Stream *stream)
{
    for(uint8_t i=0; i<MAX_STREAMS; i++) {
        if (streams[i] != NULL) {
            streams[i] = stream;
            break;
        }
    }
}

void
DBLog::flush(void)
{
    if (serialEnabled)
    {
        Serial.flush();
    }

    for(uint8_t i=0; i<MAX_STREAMS; i++) 
    {
        if (streams[i]) streams[i]->flush();
    }
}

size_t
DBLog::write(uint8_t c)
{
    size_t amtWritten;

    if (serialEnabled)
    {
        amtWritten = Serial.write(c);
    }

    for(uint8_t i=0; i<MAX_STREAMS; i++) 
    {
        if (streams[i]) amtWritten = streams[i]->write(c);
    }

    return amtWritten;
}

size_t
DBLog::write(const uint8_t *buffer, size_t size)
{
    size_t amtWritten;

    if (serialEnabled)
    {
        amtWritten = Serial.write(buffer, size);
    }

    for(uint8_t i=0; i<MAX_STREAMS; i++) 
    {
        if (streams[i]) amtWritten = streams[i]->write(buffer, size);
    }

    return amtWritten;
}

// Global instance
DBLog Log;