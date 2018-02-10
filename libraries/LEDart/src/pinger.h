#pragma once

#import "msg_tube.h"

class Pinger : public MTListener {

    uint8_t toPing;
    uint32_t pingInterval;

    uint32_t lastId;
    uint32_t lastPingAt;

    bool inFlight;
    MTMessage *msg;    
public:
    Pinger();

    void begin();
    void loop();

    void startPings(uint8_t dest, uint32_t interval);

    bool handleMTMessage(MTMessage* msg) override;
};