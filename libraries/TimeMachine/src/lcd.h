#pragma once

// #include <Esp.h>
#include <Arduino.h>
#include <Print.h>

#include <LiquidCrystal_I2C.h>
#include <log.h>
#include <bstrlib.h>

class LCD : public Print {
public:
    LCD();

    void begin();
    void loop();

    ///////////////
    // The stream interface for writing logs
    void flush(void);
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


private:
    LiquidCrystal_I2C _module;
    char _logLines[4][21];
    uint32_t _lastPing = 0;

    uint32_t _lastUpdate = 0;
    bool _needsUpdate = false;

    void displayLog();
};