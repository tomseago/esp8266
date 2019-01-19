#include "debug_buffer.h"

DebugBuffer::DebugBuffer(size_t length) :
    length(length)
{
    buffer = (char *)malloc(length+1);
    buffer[length] = 0;
    clear();
}

void
DebugBuffer::appendChar(char c)
{
    return;
    if (!end) return;

    if (end - buffer >= length -1) return;

    *end = c;
    *(end+1) = 0;
    end++;
}

void
DebugBuffer::clear()
{
    end = buffer;
    *end = 0;
}
