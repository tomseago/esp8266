#pragma once

#include "LEDArt.h"
#include "log.h"
#include "msg_tube.h"
#include "nexus_listener.h"

class WiFiSync : MTListener, NexusListener {
public:
    class StatusAnim : public LEDArtAnimation {
    public:
        StatusAnim(WiFiSync& parent);

        virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);

    private:
        WiFiSync& parent;
    };

    StatusAnim statusAnim;


    WiFiSync(Nexus& nexus);

    void begin();
    void loop();

    bool handleMTMessage(MTMessage* msg) override;

    //
    void nexusValueUpdate(NexusValueType which, uint32_t source) override;
    void nexusUserAnimationRequest(char* szName, bool randomize, uint32_t source) override;

protected:
    Nexus& nexus;
    MTMessage *stateMsg;
    MTMessage *queryMsg;

    uint32_t lastQueryAt;
    uint32_t lastId;

    bool valuesUpdated;

    void queryTime();
    void queryState();
    void sendState(); // used for normal prepared states

    // used when values update and we need an immediate thing to happen
    void sendCurrentState();
};
