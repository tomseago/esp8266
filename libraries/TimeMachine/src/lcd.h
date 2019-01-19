#pragma once

// #include <Esp.h>
#include <Arduino.h>
#include <Print.h>

#include <LiquidCrystal_I2C.h>
#include <log.h>
#include <bstrlib.h>



class LCD : public Print {
public:
    enum LCDMode {
        ModeDate = 0,
        ModeDebug,
        ModeLog,

        ModeLast
    };

    enum Button {
        BtnYear = 0,
        BtnMonth,
        BtnDay,
        BtnSel,
        BtnGo
    };

    LCD();

    void begin();
    void loop();


    void setMode(LCDMode mode);

    void setYear(uint32_t year);
    void setMonth(uint8_t month);
    void setDay(uint8_t day);
    void setSelVal(uint8_t val);

    void setButton(Button which, bool set);

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
    LiquidCrystal_I2C module;
    
    LCDMode mode;

    // Used for date display
    uint32_t year;
    uint8_t month;
    uint8_t day;

    uint8_t selVal;

    char logLines[4][21];
    uint32_t lastPing = 0;

    uint32_t lastUpdate = 0;
    bool logUpdated = false;

    void fullUpdate();

    void updateYear();
    void updateMonth();
    void updateDay();
    void updateSelVal();

    void displayLog();
};