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

    void setLeft(int32_t v);
    void setRight(int32_t v);
    void setSat(int32_t v);
    void setBright(int32_t v);

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

    int32_t left;
    int32_t right;
    int32_t sat;
    int32_t bright;

    char logLines[4][21];
    uint32_t lastPing = 0;

    uint32_t lastUpdate = 0;
    bool logUpdated = false;

    void fullUpdate();

    void updateLeft();
    void updateRight();
    void updateSat();
    void updateBright();

    void displayLog();
};