#pragma once

#import "signui.h"
#import <WebSocketsClient.h>
#import <ESP8266WiFiType.h>
#import <FS.h>

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

/*

File format for animations:

Commands:
    Set State,     State      uint16
    Delay          How long   uint16
    Jump To        location   uint16
    Random Branch   % of time to take branch, location   uint8, uint8

Units for location are cells.

An animation is an ordered sequence of cells. Each cell is 3 bytes long.
The first byte is the command. The next two bytes are data related
to that command that is intpreted within the context of the command.

Cells have an identity. They are counted and numbered. Locations of cells
are given as absolute addresses for the jump to and random branch commands.
* This could be done as relative cell addresses???


Maybe???
    Set Register
    Add To Register
    Branch if equal

*/

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

    uint32_t channelState = 0xffff;
    bool stateDirty = false;

    bool isMaster = false;

    SignUI ui;
    WebSocketsClient* wsClient = 0;

    File* currentFile = 0;
    uint32_t nextFrameAt = 0;

    uint8_t configureTries = 0;
    uint32_t resetAt = 0;

public:
    WHSign(int);

    void enableMaster();
    void enableSlave();

    void begin();
    void loop();

    void toggleChannel(uint8_t channel);
    void scheduleReset();
    void setState(uint16_t newState);

    void startAnimation(char* name);

private:
    bool animRunning = false;
    uint32_t animDelayUntil;

    uint8_t animCommand;
    uint16_t animValue;

    File animFile;

    void configurePins();
    void attemptClientConnection();
    void h_wsEvent(WStype_t type, uint8_t * payload, size_t length);

    void h_GotIP(const WiFiEventStationModeGotIP& evt);
    void h_Connected(const WiFiEventStationModeConnected& evt);
    void h_Disconnected(const WiFiEventStationModeDisconnected& evt);

    void animTick();

};

