#pragma once

// #include <ESP8266WiFi.h>

// #include <ESPAsyncTCP.h>


class HausFan {
protected:
    enum STAStatus {
        Nothing = 0,
        TryingPrimary,
        ConnectedPrimary,
        TryingSecondary,
        ConnectedSecondary,
        Disabled
    };

    STAStatus staStatus = Nothing;

    char* szAPName;
    char* szAPPassword;

    char* szSTAName[2];
    char* szSTAPassword[2];

    uint32 nextSTACheck;
    uint32 staAttemptBegan;
    uint8_t staCheckAttempts;    

    void checkSTA();
    void startSTAConnect(bool isSecondary);
    void checkSTAAttempt(bool isSecondary);
    void startSTANothingPeriod();
    void checkSTAStillOk(bool isSecondary);

public:
    HausFan();

    bool configure(const char* szAPName, const char* szAPPassword);
    void setPossibleNet(bool isSecondary, const char *szName, const char *szPassword);

    void begin();
    void loop();
};


// The global instance
extern HausFan hausFan;
