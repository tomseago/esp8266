#pragma once

#import "webui.h"
#import <WebSocketsClient.h>
#import <ESP8266WiFiType.h>

const uint8_t channel_map[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15
};

class WebSocketsClient;

class WHSign {
    enum Segments {
        Seg_Text_Welcome = 0,
        Seg_Text_Hom,
        Seg_Text_O,
        Seg_Text_E,
        Seg_Pink_Dancer,
        Seg_Pink_Dancer_Action_1,
        Seg_Pink_Dancer_Action_2,
        Seg_Pink_Fucker,
        Seg_Blue_Fucker,
        Seg_Fucker_Action_1,
        Seg_Fucker_Action_2,
        Seg_Fucker_Action_3,
        Seg_Fucker_Action_4,
        Seg_Blue_Dancer,
        Seg_Blue_Dancer_Action_1,
        Seg_Blue_Dancer_Action_2,

        Seg_LAST
    };


    enum ClientState {
        CS_DISCONNECTED = 0,
        CS_ATTEMPTING,
        CS_CONNECTED,
    };

    ClientState clientState = CS_DISCONNECTED;
    uint32_t disconnectedAt = 0;
    uint8_t consecutiveDisconnects = 0;
    uint32_t lastStateAt = 0;

    uint32_t channelState = 0;
    bool stateDirty = false;

    bool isMaster = false;

    WebUI ui;
    WebSocketsClient* wsClient = 0;

public:
    WHSign(int);

    void enableMaster();
    void enableSlave();

    void begin();
    void loop();

    void toggleChannel(uint8_t channel);

private:
    void attemptClientConnection();
    void h_wsEvent(WStype_t type, uint8_t * payload, size_t length);

    void h_GotIP(const WiFiEventStationModeGotIP& evt);
    void h_Connected(const WiFiEventStationModeConnected& evt);
    void h_Disconnected(const WiFiEventStationModeDisconnected& evt);

};

