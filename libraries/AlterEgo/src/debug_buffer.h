#pragma once

#include <Arduino.h>

class DebugBuffer {
public:
    DebugBuffer(size_t length);

    void appendChar(char c);
    void clear();
    size_t size() { return end - buffer; }

    char *buffer;
    char *end;

    size_t length;
};
