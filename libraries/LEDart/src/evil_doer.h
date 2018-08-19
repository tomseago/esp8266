#pragma once

#include "LEDArt.h"
#include "log.h"

class EvilDoer {
public:
    EvilDoer(Nexus& nexus) :
        nexus(nexus) {
    }

    void begin();
    void loop();

private:
    Nexus& nexus;

    uint32_t lastActionAt;

    uint8_t nextActionIx;

    void doNextAction();
};