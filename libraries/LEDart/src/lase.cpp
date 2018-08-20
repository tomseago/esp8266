#include "lase.h"

#include <stdio.h>

#include "log.h"

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

// const uint32_t LASE_Restart_Delay = 20000;
// const uint32_t LASE_STA_Connected_Recheck_Delay = 30000;
// const uint32_t LASE_STA_Attempt_Window = 20000;

const uint32_t PC_Restart_Delay = 5000;
const uint32_t PC_Check_Delay = 5000;
const uint32_t PC_Backoff_Delay = 20000; // want this to be big

PersistentConnection::PersistentConnection(IPAddress serverAddr, int port) :
    serverAddr(serverAddr),
    port(port)
{

}

void
PersistentConnection::begin()
{
    // Assume that something else (like haus_fan for instance) is 
    // setting up the wifi
}

void
PersistentConnection::loop()
{
    checkClient();
}


///////

void
PersistentConnection::checkClient()
{
    uint32_t now = millis();
    if (now < nextCheck)
    {
        return;
    }

    if (forceDisconnect)
    {
        forceDisconnect = false;
        WiFi.disconnect(true);
        nextCheck = now + PC_Backoff_Delay;
        return;
    }

    if (!client) {
        createClient();
        return;
    }

    // There is a client so just make sure it is ok?
    if (!client->connecting() && !client->connected()) {
        Log.printf("PC: Client in a bad state %s\n", client->stateToString());
        client = NULL;
        connectionClosed();
        nextCheck = millis() + PC_Restart_Delay;
        return;
    }

    // Seems fine
    nextCheck = millis() + PC_Check_Delay;
    return;
}

void
PersistentConnection::createClient()
{
    // Before we can start, we must be in a state of connected
    wl_status_t wifiStatus = WiFi.status();

    // This is the only thing that means it's all good
    if (WL_CONNECTED != wifiStatus) {
        Log.printf("PC: No wifi yet...\n");
        nextCheck = millis() + PC_Restart_Delay;
        return;
    }

    Log.printf("PC: Wifi is connected\n");

    // Create the local connection
    client = new AsyncClient();

    // touch it yet
    Log.printf("PC: Setting onConnect and onError handlers for new client\n");
    client->onConnect(std::bind(&PersistentConnection::cConnect, this, std::placeholders::_1, std::placeholders::_2), NULL);
    client->onError(std::bind(&PersistentConnection::cError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);

    if (!client->connect(serverAddr, port))
    {
        // Aack!
        Log.printf("PC: Unable to create AsyncClient so bailing to Nothing\n");
        client = NULL;
        connectionClosed();
        nextCheck = millis() + PC_Restart_Delay;
        return;
    }

    nextCheck = millis() + PC_Check_Delay;

    Log.printf("PC: Started TCP connection\n");
}


void
PersistentConnection::cConnect(void* context, AsyncClient* client)
{
    if (!client) return;

    Log.printf("PC: Connected to %s\n", client->remoteIP().toString().c_str());

    tcpBuffer = new AsyncTCPbuffer(client);

    tcpBuffer->onDisconnect(std::bind(&PersistentConnection::bufferDisconnect, this, std::placeholders::_1));

    // Might want to bind to reads
    //tcpBuffer->readBytes((uint8_t*)&addr, sizeof(addr), std::bind(&MTClientConnection::readAddr, this, std::placeholders::_1, std::placeholders::_2));

    // Start doing things???
    startConnection();
}

void
PersistentConnection::cError(void* context, AsyncClient* client, int8_t error)
{
    if (!client) return;

    Log.printf("PC: Error on connect %d %s\n", error, client->errorToString(error));

    // Can we nuke it here or not???

    // Nuke wifi by disconnecting when this happens. We would have only tried if
    // we thought wifi was good, but apparently it is not.
    // Wait, no! This probably just means the server isn't running!!!!
    // forceDisconnect = true;
}

bool
PersistentConnection::bufferDisconnect(AsyncTCPbuffer* obj)
{
    // Returning true from this function causes the AsyncTCPBuffer to get deleted
    if (!tcpBuffer) return true;

    Log.printf("PC: disconnect from %s\n", tcpBuffer->remoteIP().toString().c_str());

    tcpBuffer = NULL;
    client = NULL;

    return true;
}

void
PersistentConnection::writeByte(uint8_t v)
{
    tcpBuffer->write(v);
}

void
PersistentConnection::writeUInt16(uint16_t v)
{
    uint8_t* p = (uint8_t*)&v;
#ifdef I_AM_BIGENDIAN    
    tcpBuffer->write(p[0]);
    tcpBuffer->write(p[1]);
#else
    tcpBuffer->write(p[1]);
    tcpBuffer->write(p[0]);
#endif    
}

void
PersistentConnection::writeUInt32(uint32_t v)
{
    uint8_t* p = (uint8_t*)&v;
#ifdef I_AM_BIGENDIAN
    tcpBuffer->write(p[0]);
    tcpBuffer->write(p[1]);
    tcpBuffer->write(p[2]);
    tcpBuffer->write(p[3]);
#else
    tcpBuffer->write(p[3]);
    tcpBuffer->write(p[2]);
    tcpBuffer->write(p[1]);
    tcpBuffer->write(p[0]);
#endif
}

void
PersistentConnection::writeFloat(float v)
{
    uint8_t* p = (uint8_t*)&v;

#ifdef I_AM_BIGENDIAN
    tcpBuffer->write(p[0]);
    tcpBuffer->write(p[1]);
    tcpBuffer->write(p[2]);
    tcpBuffer->write(p[3]);
#else
    tcpBuffer->write(p[3]);
    tcpBuffer->write(p[2]);
    tcpBuffer->write(p[1]);
    tcpBuffer->write(p[0]);
#endif
}

void
PersistentConnection::writeCStr(char* sz)
{
    tcpBuffer->write(sz);
    tcpBuffer->write((uint8_t)0);
}

/////////////

const uint32_t Lase_Value_Interval = 1000;
const uint32_t Lase_Pixel_Interval = 100;

Lase::Lase(IPAddress serverAddr, uint8_t id, Nexus &nexus, LEDArtPiece &piece) :
    PersistentConnection(serverAddr, 7453),
    id(id),
    nexus(nexus),
    piece(piece),
    pixelsFrameRate(20)
{

}

void
Lase::begin()
{
    PersistentConnection::begin();
}

void
Lase::loop()
{
    PersistentConnection::loop();

    // If the client isn't happy, don't send anything
    if (client==NULL || !client->connected()) return;

    uint32_t now = millis();
    switch(laseState)
    {
    case Initial:
        sendSetup();
        laseState = Labels;
        return;

    case Labels:
        sendLabels();
        laseState = Normal;
        nextValueTime = 0;
        nextPixelTime = 0;
        return;

    case Normal:
        if (now > nextValueTime)
        {
            sendValues();
            nextValueTime = now + Lase_Value_Interval;
        }

        if (now > nextPixelTime)
        {
            sendPixels();
            nextPixelTime = now + Lase_Pixel_Interval;
        }
        return;
    }
}

void 
Lase::startConnection()
{
    // Setup the on receive data handlers if we care about data (right now we do not)
    laseState = Initial;
}

void
Lase::connectionClosed()
{
    // Lovely?
    // client will be null so don't have to update the state
}

void
Lase::sendSetup()
{
    Log.printf("LASE: sendSetup\n");
    if (!tcpBuffer) return;

    // Message Type 2
    // uint16 pieceNumber
    // uint16 numPixels
    // uint8  bytesPerPixel
    writeByte(2);
    writeUInt16(id);
    writeUInt16(piece.strip.PixelCount());
    writeByte(piece.strip.PixelSize());
}

void
Lase::sendLabels()
{
    Log.printf("LASE: sendLabels\n");

    // Message Type 3 labels
    // uint8   state variable tag, 0 indicates end of message
    // string  label    
    writeByte(3);

    writeByte(1);
    writeCStr("Now");

    writeByte(2);
    writeCStr("TO");

    writeByte(3);
    writeCStr("Anim");

    writeByte(4);
    writeCStr("GeomId");

    writeByte(5);
    writeCStr("When");

    writeByte(6);
    writeCStr("N Anim");

    writeByte(7);
    writeCStr("SQE");

    writeByte(8);
    writeCStr("Pixels @");

    writeByte(0);
}

void
Lase::sendValues()
{
    //Log.printf("LASE: sendValues\n");

    // Message type 4, value data
    // uint8   state variable tag, 0 indicates end of message
    // uint8   state variable type, 0 string, 1 int32, 2 float
    // data for type
    writeByte(4);

    uint32_t now = millis() + nexus.localTimeOffset;
    writeByte(1);
    //writeCStr("TO");
    writeByte(1);
    writeUInt32(now);

    writeByte(2);
    //writeCStr("TO");
    writeByte(1);
    writeUInt32(nexus.localTimeOffset);

    writeByte(3);
    //writeCStr("Anim");
    writeByte(0);
    writeCStr(nexus.getCurrentAnimName());

    writeByte(4);
    //writeCStr("GeomId");
    writeByte(1);
    writeUInt32(piece.geomId());

    uint32_t when;
    char* szGeomName;
    bool geomRotated;
    char* szAnimName;

    when = nexus.nextPreparedState(&szGeomName, &geomRotated, &szAnimName);

    writeByte(5);
    //writeCStr("When");
    writeByte(1);
    writeUInt32(when);

    writeByte(6);
    //writeCStr("N Anim");
    writeByte(0);
    writeCStr(szAnimName ? szAnimName : (char*)"NULL");

    writeByte(7);
    //writeCStr("SQE");
    writeByte(1);
    writeUInt32(nexus.stateQEvents);

    writeByte(8);
    //writeCStr("Pixels");
    writeByte(1);
    writeUInt32(pixelsFrameRate.avgIFTms());

    writeByte(0);    
}

void 
Lase::sendPixels()
{
    //Log.printf("LASE: sendPixels\n");

    // Message type 1, pixel data
    writeByte(1);
    tcpBuffer->write(piece.strip.Pixels(), piece.strip.PixelsSize());

    pixelsFrameRate.mark();
}
