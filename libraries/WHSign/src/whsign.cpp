#import "whsign.h"
#include <functional>

extern "C" {
#include "user_interface.h"
}


// static const uint8_t D0   = 16;
// static const uint8_t D1   = 5;
// static const uint8_t D2   = 4;
// static const uint8_t D3   = 0;
// static const uint8_t D4   = 2;
// static const uint8_t D5   = 14;
// static const uint8_t D6   = 12;
// static const uint8_t D7   = 13;
// static const uint8_t D8   = 15;
// static const uint8_t RX   = 3;
// static const uint8_t TX   = 1;

bool haveIP = false;

void(* crashNow)(void) = 0;//declare reset function at address 0

void h_WiFiEvent(WiFiEvent event) 
{
    Serial.printf("[ME ME ME] event: %d\n", event);

    switch(event) {
        case WIFI_EVENT_STAMODE_CONNECTED:
            Serial.printf("STA: Wifi connected\n");
            break;

        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Serial.printf("STA: Wifi disconnected\n");
            haveIP = false;
            break;

        case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
            Serial.printf("STA: Auth mode change\n");
            break;

        case WIFI_EVENT_STAMODE_GOT_IP:
            Serial.printf("STA: Got IP\n");
            Serial.println(WiFi.localIP());
            haveIP = true;
            break;

        case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
            Serial.printf("STA: DHCP Timeout\n");
            break;

        case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
            Serial.printf("AP: Station connected\n");
            break;

        case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
            Serial.printf("AP: Station disconnected\n");
            break;

        case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
            Serial.printf("AP: Probe request\n");
            break;
    }
}


WHSign::WHSign(int a) : 
    ui(*this)
{
}

void
WHSign::enableMaster()
{
    isMaster = true;
}

void
WHSign::enableSlave()
{
    isMaster = false;

    wsClient = new WebSocketsClient();
}

void
WHSign::begin()
{
    Serial.print("Begin... ");
    WiFi.onEvent(h_WiFiEvent);
    // WiFi.onEvent(std::bind(&WHSign::h_WiFiEvent, this, std::placeholders::_1), WIFI_EVENT_ANY);

    // WiFi.onStationModeGotIP(std::bind(&WHSign::h_GotIP, this, std::placeholders::_1));
    // WiFi.onStationModeConnected(std::bind(&WHSign::h_Connected, this, std::placeholders::_1));
    // WiFi.onStationModeDisconnected(std::bind(&WHSign::h_Disconnected, this, std::placeholders::_1));
        
    ui.begin();

    configurePins();

    if (wsClient) 
    {
        wsClient->onEvent(std::bind(&WHSign::h_wsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        attemptClientConnection();

        Serial.printf("Started wsClient (hopefully)\n");
    }
}

int seconds = 0;

void
WHSign::loop()
{
    // Check for reset condition
    if (resetAt != 0 && resetAt < millis()) {
        resetAt = 0;
        system_restart();
        return;
    }


    // Every 1 second at a minimum update the state
    int m = millis() / 1000;
    if (m>seconds) {
        seconds = m;
        stateDirty = true;
        // Serial.printf("%d ", seconds);

        // if (seconds % 10 == 0) {
        //     Serial.print("\n");
        // }
    }    


    if (wsClient) {
        if (clientState == CS_DISCONNECTED && (millis() - disconnectedAt > 2000) ) {
            // Try again!
            attemptClientConnection();
        } else if (clientState == CS_CONNECTED && (millis() - lastStateAt > 5000) ) {
            // Disconnect because it's been to long. We assume things are bad.
            Serial.printf("Too long without a state. Disconnecting\n");
            wsClient->disconnect();
        }
    }

    if (stateDirty) {
        Serial.printf("State=%04x\n", channelState);

        if (configureTries < 50) {
            configurePins();
        }

        // Output the state on our pins
        if (isMaster) {
            // The master output map
            digitalWrite(D0, channelState & (1 << 0) ? 1 : 0);
            digitalWrite(D1, channelState & (1 << 1) ? 1 : 0);
            digitalWrite(D2, channelState & (1 << 2) ? 1 : 0);
            digitalWrite(D3, channelState & (1 << 3) ? 1 : 0);
            digitalWrite(D5, channelState & (1 << 4) ? 1 : 0);
            digitalWrite(D6, channelState & (1 << 5) ? 1 : 0);
            digitalWrite(D7, channelState & (1 << 6) ? 1 : 0);
            digitalWrite(D8, channelState & (1 << 7) ? 1 : 0);
        } else {
            // The slave output map
            digitalWrite(D0, channelState & (1 << 8) ? 1 : 0);
            digitalWrite(D1, channelState & (1 << 9) ? 1 : 0);
            digitalWrite(D2, channelState & (1 <<10) ? 1 : 0);
            digitalWrite(D3, channelState & (1 <<11) ? 1 : 0);
            digitalWrite(D5, channelState & (1 <<12) ? 1 : 0);
            digitalWrite(D6, channelState & (1 <<13) ? 1 : 0);
            digitalWrite(D7, channelState & (1 <<14) ? 1 : 0);
            digitalWrite(D8, channelState & (1 <<15) ? 1 : 0);
        }

        // Broadcast the state to everyone who cares
        ui.broadcastState(channelState);

        stateDirty = false;
    }
}

void
WHSign::toggleChannel(uint8_t channel)
{
    uint16_t mask = 0x01 << channel;

    channelState ^= mask;

    stateDirty = true;
}

void
WHSign::scheduleReset()
{
    resetAt = millis() + 500;

    // Tell any client to reset
    ui.broadcastReset();
}

void
WHSign::configurePins()
{
    Serial.printf("*** Configure Pins ***\n");

    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
    pinMode(D8, OUTPUT);

    configureTries++;    
}

void
WHSign::attemptClientConnection()
{
    if (!wsClient || !haveIP) return;

    if (clientState == CS_CONNECTED)
    {
        Serial.printf("Not trying to connect when already connected\n");
        return;
    }

    
    clientState = CS_ATTEMPTING;
    wsClient->begin("192.168.10.10", 80, "/socket");
}

void
WHSign::h_wsEvent(WStype_t type, uint8_t * payload, size_t length) 
{

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WSc] Disconnected!\n");
            clientState = CS_DISCONNECTED;
            disconnectedAt = millis();
            consecutiveDisconnects++;

            if (consecutiveDisconnects > 4) {
                consecutiveDisconnects = 0;
                Serial.printf("\n\nOh gawd. Restart....\n");
                delay(200);
                Serial.printf("%5f", consecutiveDisconnects/0);
                //crashNow();
            }
            break;

        case WStype_CONNECTED:
            {
                consecutiveDisconnects = 0;
                disconnectedAt = -1;
                clientState = CS_CONNECTED;
                Serial.printf("[WSc] Connected to url: %s\n",  payload);
                
                // send message to server when Connected
                wsClient->sendTXT("Hiiiiiii!");                

                // We don't _really_ have the state but pretend...
                lastStateAt = millis();
            }
            break;
        case WStype_TEXT:
            Serial.printf("[WSc] got text: %s\n", payload);
            if (length==5) {
                if (strcmp((const char*)payload, "RESET") == 0) {
                    os_printf("******* System Restart *********\n");
                    system_restart();
                    return;
                }
            }


            break;
        case WStype_BIN:
            Serial.printf("[WSc] got binary length: %u\n", length);
            //hexdump(payload, length);

            if (length==2) {
                channelState = *((uint16_t*)payload);
                Serial.printf("channelState is now %d\n", channelState);

                stateDirty = true;
                lastStateAt = millis();
            }
            break;
    }

}




void
WHSign::h_GotIP(const WiFiEventStationModeGotIP& evt)
{
    Serial.printf("******* Got IP\n");
}

void
WHSign::h_Connected(const WiFiEventStationModeConnected& evt)
{
    Serial.printf("******* Connected\n");
}

void
WHSign::h_Disconnected(const WiFiEventStationModeDisconnected& evt)
{
    Serial.printf("******* Disconnected\n");
}
