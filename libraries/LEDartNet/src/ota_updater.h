#pragma once

#include <Arduino.h>

#include <WiFiUdp.h>

const uint16_t OtaTriggerPort = 7777;


class OtaUpdater {

    uint32_t _version; // my current firmware version
    WiFiUDP udp;

public:
    OtaUpdater(uint32_t version) :
        _version(version)
    {

    }

    void begin();

    void loop();
};

