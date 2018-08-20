#pragma once

#include <Arduino.h>

class BufferQ {
public:
    BufferQ() : head(NULL) { }

    bool hasBuffers();
    uint8_t* peek(size_t* len);
    bool pop();

    // Copies something in using strndup to make sure it
    // is NULL terminated
    bool pushCStr(char* str, size_t max);

    // Copies the data exactly
    bool pushData(uint8_t* data, size_t len);

    bool pushNoCopy(uint8_t* data, size_t len);

private:
    struct Buffer {
        uint8_t* data;
        size_t len;

        Buffer* next;
    };

    Buffer* head;
    Buffer* tail;
};