#include <stdio.h>

#include "haus_fan.h"
#include "log.h"
#include "node_config.h"

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <ESPAsyncTCPbuffer.h>

const uint32_t HF_STA_Restart_Delay = 20000;
const uint32_t HF_STA_Connected_Recheck_Delay = 30000;
const uint32_t HF_STA_Attempt_Window = 20000;


/////////////////////////////////////////////////////////////////////////////////////

HausFan::HausFan() 
{

}

void
HausFan::setPossibleNet(bool isSecondary, const char *szName, const char *szPassword)
{
    this->szSTAName[isSecondary ? 1 : 0] = (char *)szName;
    this->szSTAPassword[isSecondary ? 1 : 0] = (char *)szPassword;
}


void
HausFan::begin()
{
    // Don't worry about storing things in flash
    WiFi.persistent(false);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    //------ Configure the AP side of things

    Log.printf("HF: Starting soft AP %s %s\n", NodeConfig.wifiBaseName(), NodeConfig.wifiPassword());
    WiFi.softAP(NodeConfig.wifiBaseName(), NodeConfig.wifiPassword());


    IPAddress localIp = NodeConfig.staticMasterIP();
    IPAddress gateway = NodeConfig.staticGateway();
    IPAddress subnet(255, 255, 255, 0);

    // localIp[2] = nodeId + 10;
    // // localIp[3] = nodeId + 10;
    // gateway = localIp;


    Log.printf("HF: Calling softAPConfig(%s, %s, %s)\n", localIp.toString().c_str(), gateway.toString().c_str(), subnet.toString().c_str());
    WiFi.softAPConfig(localIp, gateway, subnet);


    //------- Setup our server for receiving connections
    // server.onClient(std::bind(&MsgTube::sConnect, this, std::placeholders::_1, std::placeholders::_2), NULL);
    // server.begin();

    // Log.printf("MT: Started server on port %d\n", MTServerPort);

    //------- Start trying to connect to upstream peers
}

void
HausFan::loop()
{
    checkSTA();
}


///////

void
HausFan::checkSTA()
{
    uint32_t now = millis();
    if (now < nextSTACheck)
    {
        return;
    }

    //Log.printf("MT: STA check in state %d\n", staStatus);

    switch(staStatus) {
    case Nothing:
        startSTAConnect(false);
        break;

    case TryingPrimary:
        checkSTAAttempt(false);
        break;

    case TryingSecondary:
        checkSTAAttempt(true);
        break;

    case ConnectedPrimary:
        checkSTAStillOk(false);
        break;

    case ConnectedSecondary:
        checkSTAStillOk(true);

    case Disabled:
        // Do nothing
        break;
    }
}

void
HausFan::startSTAConnect(bool isSecondary)
{    
    const char* szName = this->szSTAName[isSecondary ? 1 : 0];
    const char* szPassword = this->szSTAPassword[isSecondary ? 1 : 0];

    Log.printf("HF: startSTAConnect(isSecondary=%s) szName=%s\n", isSecondary ? "true": "false", szName ? szName : "NULL");

    if (!szName) 
    {
        // There is nothing configured so go to nothing
        startSTANothingPeriod();
        return;
    }

    // Start by making sure we are consistently disconnected
    Log.printf("HF: SSC WiFi.disconnect(true)\n");
    WiFi.disconnect(true);

    Log.printf("HF: SSC WiFi.begin(%s,%s)\n", szName, szPassword);
    WiFi.begin(szName, szPassword);
    Log.printf("HF: SSC WiFi.begin() returned\n");

    // Start checking status to see if we get connected
    staAttemptBegan = millis();
    staStatus = isSecondary ? TryingSecondary : TryingPrimary;
    nextSTACheck = staAttemptBegan + 2000;
    staCheckAttempts = 0;
    Log.printf("HF: SSC staAttemptBegan=%d now we wait...\n", staAttemptBegan);
}

void
HausFan::checkSTAAttempt(bool isSecondary)
{
    wl_status_t wifiStatus = WiFi.status();

    staCheckAttempts++;
    if (staCheckAttempts == 1)
    {
        // First time
        Log.printf("HF: checkSTAAttempt(isSecondary=%s) wifiStatus=%d\n", isSecondary ? "true": "false", wifiStatus);
    }
    else
    {        
        Log.printf("HF: staCheckAttempts=%d, wifiStatus=%d\n", wifiStatus);
    }

    if (staCheckAttempts == 40)
    {
        // Log.print("\n");
        staCheckAttempts = 1;
    }
    nextSTACheck = millis() + 2000;

/*
~/Library/Arduino15/packages/esp8266/hardware/esp8266/2.4.1/libraries/ESP8266WiFi/src/include/wl_definitions.h

typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;
*/

    // This is the only thing that means it's all good
    if (WL_CONNECTED == wifiStatus) {
        Log.printf("HF: Connected as a station!!!\n");
        staStatus = isSecondary ? ConnectedSecondary : ConnectedPrimary;
        nextSTACheck = millis() + HF_STA_Connected_Recheck_Delay;

        // // Create the local connection
        // AsyncClient* client = new AsyncClient();
        // IPAddress peer(10,7,10,1);
        // peer[2] += isSecondary ? nodeId - 2 : nodeId - 1;

        // // touch it yet
        // Log.printf("MT: Setting onConnect and onError handlers for new client\n");
        // client->onConnect(std::bind(&MsgTube::sConnect, this, std::placeholders::_1, std::placeholders::_2), NULL);
        // client->onError(std::bind(&MsgTube::conError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);

        // if (!client->connect(peer, 2000))
        // {
        //     // Aack!
        //     Log.printf("MT: Unable to create AsyncClient so bailing to Nothing\n");
        //     startSTANothingPeriod();
        //     return;
        // }

        // // MTClientConnection* conn = new MTClientConnection();
        // // if (!conn->begin(client))
        // // {
        // //     Log.printf("MT: Failed to create MTClientConnection for STA");
        // //     delete conn;
        // // }
        // // addConnection(conn);

        // Log.printf("MT: New client connection for STA. Yay!!");

        return;
    }

    // Anything that is not an error is just "disconnected"
    if (WL_DISCONNECTED == wifiStatus) {
        // How long has it been?
        if (millis() - staAttemptBegan < HF_STA_Attempt_Window) {
            // This is cool, let it keep trying
            Log.printf("HF: WL_DISCONNECTED but inside %d is inside window of %d, waiting...\n", millis() - staAttemptBegan, HF_STA_Attempt_Window);
            return;
        }

        // Aack, timeout. That's the same as an error
        Log.printf("HF: WL_DISCONNECTED and window exceeded. New attempt!\n");
     }

    // Some sort of error has occurred, likely state = 1 WL_NO_SSID_AVAIL

    Log.printf("HF: Giving up on this connect attempt, starting a new one\n");

    if (!isSecondary)
    {
        // Move on to secondary
        startSTAConnect(true);
    }
    else
    {
        // It was secondary, so now we must delay
        startSTANothingPeriod();
    }
}

void
HausFan::startSTANothingPeriod()
{
    Log.printf("HF: startSTANothingPeriod()\n");
    staStatus = Nothing;
    nextSTACheck = millis() + HF_STA_Restart_Delay;    
}

void
HausFan::checkSTAStillOk(bool isSecondary)
{
    wl_status_t wifiStatus = WiFi.status();

    Log.printf("HF: checkSTAStillOk(isSecondary=%s) wifiStatus=%d\n", isSecondary ? "true": "false", wifiStatus);
    // This is the only thing that means it's all good
    if (WL_CONNECTED == wifiStatus) {
        if (isSecondary)
        {
            // Let's try to connect to Primary. This will probably
            // cause a disruption in network traffic, but eh???
            startSTAConnect(false);
            return;
        }

        // else it's connected to primary, so leave it be
        nextSTACheck = millis() + HF_STA_Connected_Recheck_Delay;
        return;
    }

    // Oh golly, anything else sucks, so we need to re-connect.
    // If we were connected to primary and it failed, does that mean
    // we should immediately try secondary or not? For now, just
    // always restart at nothing.
    staStatus = Nothing;
}

// A global instance
HausFan hausFan;