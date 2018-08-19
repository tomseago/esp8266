#pragma once

// #include <ESP8266WiFi.h>
#include <WiFi.h>

#ifdef ESP32
#include <AsyncTCP.h>
#else
#include <ESPAsyncTCP.h>
#endif

const uint16_t MTServerPort = 2000;
const uint8_t MTBroadcastAddr = 255;

const uint8_t MTMasterAddr = 1;

const uint8_t MT_Type_Generic = 0;
const uint8_t MT_Type_Log = 1;
const uint8_t MT_Type_Ping = 2;
const uint8_t MT_Type_Pong = 3;

// WiFiSync
const uint8_t MT_Type_TimeQuery = 4;
const uint8_t MT_Type_TimeResponse = 5;
const uint8_t MT_Type_StateQuery = 6;
const uint8_t MT_Type_PreparedState = 7;
const uint8_t MT_Type_CurrentState = 8;


class MTMessage;
class MsgTube;
class MTListener;

// Internal ones
class MTClientConnection;


class MTListener {
    friend MsgTube;

protected:
    // To be used by     
    MTListener* nextMTListener = NULL;

public:
    // Returns true if handled so that a listener chain can be created
    virtual bool handleMTMessage(MTMessage* msg) = 0;
};


class MTMessage {
    friend MsgTube;
    friend MTClientConnection;

protected:
    static uint32_t nextIdVal;

    MTMessage* next = NULL;
    MTClientConnection* conn = NULL;

    uint32_t id;
    uint8_t src;
    uint8_t dest;
    uint8_t type;

    uint16_t capacity;
    uint16_t length;
    uint8_t* data;

    MTMessage(uint16_t capacity);
    ~MTMessage();

    void readFrom(MTClientConnection* conn);
    void sendTo(MTClientConnection* conn);

    void readIdDone(bool ok, void * ret);
    void readSrcDone(bool ok, void * ret);
    void readDestDone(bool ok, void * ret);
    void readTypeDone(bool ok, void * ret);
    void readDataLenDone(bool ok, void * ret);
    void readDataDone(bool ok, void * ret);

public:
    bool isBroadcast() { return dest == MTBroadcastAddr; }

    uint32_t getId() { return id; }
    uint8_t getSrc() { return src; }
    uint8_t getDest() { return dest; }
    uint8_t getType() { return type; }

    uint16_t getCapacity() { return capacity; }
    uint16_t getLength() { return length; }
    const uint8_t* getData() { return (const uint8_t*)data; }

    void incrementId() { id = ++nextIdVal; }
    void setId(uint32_t idIn) { id = idIn; }
    void setSrc(uint8_t srcIn) { src = srcIn; }    
    void setDest(uint8_t destIn) { dest = destIn; }
    void swapSrcDest() { uint8_t t = dest; dest = src; src = t; }
    void setType(uint8_t tc) { type = tc; }

    // Copies the data into the message
    void setData(uint8_t* data, uint16_t len);

    // Appends data to the message
    void catData(uint8_t* data, uint16_t len);
};

class MsgTube {
protected:
    enum STAStatus {
        Nothing = 0,
        TryingPrimary,
        ConnectedPrimary,
        TryingSecondary,
        ConnectedSecondary,
        Disabled,

        TryingStatic,
        ConnectedStatic,
    };

    STAStatus staStatus = Nothing;    

    uint8_t nodeId;
    char* szBaseName;
    char* szPassword;
    bool isStaticMode;

    MTListener* listeners = NULL;

    AsyncServer server;

    MTClientConnection* clientConns = NULL;

    void sConnect(void* context, AsyncClient* client);
    void conError(void* context, AsyncClient* client, int8_t error);
    void addConnection(MTClientConnection* conn);

    void dumpClientConns();

    uint32_t nextSTACheck;
    uint32_t staAttemptBegan;
    uint8_t staCheckAttempts;

    void checkSTA();
    void startSTAConnect(bool isSecondary);
    void checkSTAAttempt(bool isSecondary);
    void startSTANothingPeriod();
    void checkSTAStillOk(bool isSecondary);

    void startStaticConnect();
    void checkStaticAttempt();
    void checkStaticStillOk();

    void handleMessage(MTMessage* msg);

public:
    MsgTube();

    // Before calling begin set config data
    bool configure(const uint8_t nodeId, const char* szBaseName, const char* szPassword);

    // Default mode is mesh, but this will put us into static mode (one time config before
    // calling begin)
    void enableStatic();

    void begin();
    void loop();

    uint8_t getNodeId() { return nodeId; }
    bool isMaster() { return nodeId == MTMasterAddr; }

    void addListener(MTListener* listener);

    // Use this to create all new messages. The returned message object is owned
    // by the MsgTube.
    MTMessage* newMessage(uint16_t capacity);

    // Send a previously created message. After this is called the message object
    // is under the control of the tube and should not be modified.
    void sendMessage(MTMessage* msg);


    ////////////
    void connectionClosed(MTClientConnection* conn);
    void receivedMessage(MTClientConnection* conn, MTMessage* msg);
};


extern MsgTube msgTube;