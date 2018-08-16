#pragma once

#include "Arduino.h"
#include <Print.h>
#include <Stream.h>

class DBLog: public Print {

public:
    DBLog();

    void setSerialEnabled(bool en);
    bool addPrint(Print *stream);
    void removePrint(Print *stream);

    void flush(void) override;
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    inline size_t write(unsigned long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t) n);
    }
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

extern DBLog Log;


class FPSCounter {

public:
    uint32_t* interFrameTimes;
    uint8_t capacity;
    uint8_t nextSlot;
    uint32_t lastFrameAt;

    FPSCounter(uint8_t window) {
        interFrameTimes = (uint32_t*)malloc(window * sizeof(uint32_t));
        memset(interFrameTimes, 0, window * sizeof(uint32_t));

        capacity = window;        
    }

    void mark() {
        uint32_t now = millis();
        uint32_t elapsed = now - lastFrameAt;

        interFrameTimes[nextSlot] = elapsed;
        nextSlot++;
        if (nextSlot == capacity) {
            nextSlot = 0;
        }

        lastFrameAt = now;
    }

    uint32_t avgIFTms() {
        uint32_t accum = 0;

        for(int i=0; i<capacity; i++) {
            accum += interFrameTimes[i];
        }
        return accum / capacity;
    }

    float avgFPS() {
        return 1000.0 / (float)avgIFTms();
    }
};
