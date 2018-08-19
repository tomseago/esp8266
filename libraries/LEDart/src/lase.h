#pragma once

#include "LEDArt.h"
#include "nexus.h"
#include "log.h"


#ifdef ESP32
#include <AsyncTCP.h>
#else
#include <ESPAsyncTCP.h>
#endif

#include <ESPAsyncTCPbuffer.h>


class PersistentConnection {
protected:
    IPAddress serverAddr;
    int port;

    AsyncClient* client;
    AsyncTCPbuffer* tcpBuffer;
    uint32_t nextCheck;

    bool forceDisconnect;


    void cConnect(void* context, AsyncClient* client);
    void cError(void* context, AsyncClient* client, int8_t error);

    bool bufferDisconnect(AsyncTCPbuffer* obj);

    void checkClient();
    void createClient();

    virtual void startConnection() { };
    virtual void connectionClosed() { };

    // These all send BigEndian because networks
    void writeByte(uint8_t v);
    void writeUInt16(uint16_t v);
    void writeUInt32(uint32_t v);
    void writeFloat(float v);
    void writeCStr(char* sz); // Adds the nul for you

public:
    PersistentConnection(IPAddress serverAddr, int port);

    virtual void begin();
    virtual void loop();
};

class Lase : public PersistentConnection{
protected:
    enum LaseState {
        Initial = 0,
        Labels,
        Normal
    };
    LaseState laseState = Initial;

    uint32_t nextValueTime;
    uint32_t nextPixelTime;    

    uint8_t id;
    LEDArtPiece &piece;
    Nexus &nexus;

    virtual void startConnection();
    virtual void connectionClosed();

    FPSCounter pixelsFrameRate;

    void sendSetup();
    void sendLabels();
    void sendValues();
    void sendPixels();

public:
    Lase(IPAddress serverAddr, uint8_t id, Nexus &nexus, LEDArtPiece &piece);

    virtual void begin();
    virtual void loop();

};