#import "whsign.h"
#include <functional>

#import <rand.h>

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
    // Serial.printf("[h_WiFiEvent] event: %d\n", event);

    switch(event) {
        case WIFI_EVENT_STAMODE_CONNECTED:
            Serial.printf("~STA: Wifi connected\n");
            break;

        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Serial.printf("~STA: Wifi disconnected\n");
            haveIP = false;
            break;

        case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
            Serial.printf("~STA: Auth mode change\n");
            break;

        case WIFI_EVENT_STAMODE_GOT_IP:
            Serial.printf("~STA: Got IP\n");
            Serial.println(WiFi.localIP());
            haveIP = true;
            break;

        case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
            Serial.printf("~STA: DHCP Timeout\n");
            break;

        case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
            Serial.printf("~AP: Station connected\n");
            break;

        case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
            Serial.printf("~AP: Station disconnected\n");
            break;

        case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
            Serial.printf("~AP: Probe request\n");
            break;
    }
}


WHSign::WHSign(int a) : 
    ui(*this)
{
    disconnectedAt = millis();
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

        //attemptClientConnection();

        //Serial.printf("~Started wsClient (hopefully)\n");
    }

    // Start default animation
    startAnimation(NULL);
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

    // Do any animation that should be happening
    animTick();

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
            Serial.printf("~Too long without a state. Disconnecting\n");
            wsClient->disconnect();
        }

        // A little unnecessary with async but actually necessary
        //Serial.printf("~Calling client loop()\n");
        wsClient->loop();
        //Serial.printf("~Done with client loop()\n");
    }

    if (stateDirty) {
        Serial.printf("~%c State=%04x\n", (isMaster ? 'M' : 's'), channelState);

        // if (configureTries < 50) {
        //     configurePins();
        // }

        // Output the state on our pins
        if (isMaster) {
            // The master output map
            digitalWrite(D0, channelState & (1 << 0) ? 1 : 0); //  0
            digitalWrite(D1, channelState & (1 << 9) ? 1 : 0); //  1
            digitalWrite(D2, channelState & (1 << 4) ? 1 : 0); //  2
            digitalWrite(D3, channelState & (1 << 8) ? 1 : 0); //  3
            digitalWrite(D5, channelState & (1 << 7) ? 1 : 0); //  4
            digitalWrite(D6, channelState & (1 << 5) ? 1 : 0); //  5
            digitalWrite(D7, channelState & (1 <<13) ? 1 : 0); //  6
            digitalWrite(D8, channelState & (1 <<14) ? 1 : 0); //  7
        } else {
            // The slave output map
            digitalWrite(D0, channelState & (1 << 3) ? 1 : 0); //  8
            digitalWrite(D1, channelState & (1 <<12) ? 1 : 0); //  9
            digitalWrite(D2, channelState & (1 << 1) ? 1 : 0); // 10
            digitalWrite(D3, channelState & (1 << 2) ? 1 : 0); // 11
            digitalWrite(D5, channelState & (1 << 6) ? 1 : 0); // 12
            digitalWrite(D6, channelState & (1 <<11) ? 1 : 0); // 13
            digitalWrite(D7, channelState & (1 <<15) ? 1 : 0); // 14
            digitalWrite(D8, channelState & (1 <<10) ? 1 : 0); // 15
        }

        // Broadcast the state to everyone who cares
        if (isMaster) ui.broadcastState(channelState);

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
WHSign::setState(uint16_t newState)
{
    channelState = newState;
    Serial.printf("~setState(%04x)\n", channelState);

    stateDirty = true;
    lastStateAt = millis();
}

void
WHSign::scheduleReset()
{
    Serial.printf("~******* Schedule reset\n");
    resetAt = millis() + 500;

    // Tell any client to reset
    ui.broadcastReset();
}

void
WHSign::configurePins()
{
    Serial.printf("~*** Configure Pins ***\n");

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
    // Always reset this so we don't freak out.
    disconnectedAt = millis();

    if (!wsClient) {
        Serial.println("~No wsClient. Not connect attempt");
        return;
    }

    if (!haveIP) {
        Serial.println("~Don't have an ip. Won't try to connect");
        return;
    }

    if (clientState == CS_CONNECTED)
    {
        Serial.printf("~Not trying to connect when already connected\n");
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
            if (clientState == CS_DISCONNECTED) {
                // We know, we don't care
                return;
            }
            Serial.printf("~[WSc] Disconnected!\n");
            disconnectedAt = millis();
            clientState = CS_DISCONNECTED;
            //clientState = CS_DISCONNECTED;
            //disconnectedAt = millis();
            //consecutiveDisconnects++;

            // if (consecutiveDisconnects > 4) {
            //     consecutiveDisconnects = 0;
            //     Serial.printf("\n\n~Oh gawd. Restart....\n");
            //     delay(200);
            //     Serial.printf("%5f", consecutiveDisconnects/0);
            //     //crashNow();
            // }
            break;

        case WStype_CONNECTED:
            {
                consecutiveDisconnects = 0;
                disconnectedAt = -1;
                clientState = CS_CONNECTED;
                Serial.printf("~[WSc] Connected to url: %s\n",  payload);
                
                // send message to server when Connected
                wsClient->sendTXT("Hiiiiiii!");                

                // We don't _really_ have the state but pretend...
                lastStateAt = millis();
            }
            break;
        case WStype_TEXT:
            Serial.printf("~[WSc] got text: %s\n", payload);
            if (length==5) {
                if (strcmp((const char*)payload, "RESET") == 0) {
                    os_printf("~******* System Restart *********\n");
                    system_restart();
                    return;
                }
            }


            break;
        case WStype_BIN:
            Serial.printf("~[WSc] got binary length: %u\n", length);
            //hexdump(payload, length);

            if (length==2) {
                setState(*((uint16_t*)payload));
            }
            break;
    }

}




void
WHSign::h_GotIP(const WiFiEventStationModeGotIP& evt)
{
    Serial.printf("~~~~******* Got IP\n");
}

void
WHSign::h_Connected(const WiFiEventStationModeConnected& evt)
{
    Serial.printf("~~~******* Connected\n");
}

void
WHSign::h_Disconnected(const WiFiEventStationModeDisconnected& evt)
{
    Serial.printf("~~~******* Disconnected\n");
}



///////////////////////

void
WHSign::animTick()
{
    if (!animRunning) {
        return;
    }

    if (!animFile) {
        return;
    }

    if (animDelayUntil > millis()) {
        // Serial.printf("waiting until %d (now %d)\n", animDelayUntil, millis());
        return;
    }

    // Time for next command
    if (animFile.position() == animFile.size()) {
        // Loop to beginning of file
        animFile.seek(0, SeekSet);
    }

    // Serial.printf("Attempting read\n");
    uint8_t cmdBuf[3];
    size_t amount = animFile.read(cmdBuf, 3);
    if (amount != 3) {
        Serial.println("Failed to read 3 bytes from file. Stopping");
        animRunning = false;
        return;
    }

    uint16_t val = (((uint16_t)cmdBuf[1]) << 8) + (uint16_t)cmdBuf[2];

    Serial.printf("Read command 0x%02x%02x%02x. val=%d\n", cmdBuf[0], cmdBuf[1], cmdBuf[2], val);
    // animRunning = false;
    // return;


    switch(cmdBuf[0]) {
        case 0: // Set
            setState(val);
            break;

        case 1: // Delay
            animDelayUntil = millis() + val;
            Serial.printf("Anim delay until %d (now %d)\n",animDelayUntil, millis());
            break;

        case 2: // Jump
            {
                uint32_t pos = val * 3;
                if (pos > animFile.size()-3) {
                    Serial.printf("Attempt to jump to %d but size is %d. Stopping.\n", pos, animFile.size());
                    animRunning = false;                
                } else {
                    Serial.printf("Jump to %d\n", val);
                    animFile.seek(pos, SeekSet);
                }
            }
            break;

        case 3: // Random jump
            {
                uint8_t rVal = rand(255);

                if (rVal < cmdBuf[1]) {
                    // Jump!!!
                    uint32_t pos = cmdBuf[2] * 3;
                    if (pos > animFile.size() - 3) {
                        Serial.printf("Random jump to %d but size is %d. Stopping.\n", pos, animFile.size());
                        animRunning = false;
                    } else {
                        Serial.printf("Random jump to %d\n", cmdBuf[2]);
                        animFile.seek(pos, SeekSet);
                    }
                } else {
                    // No jump, do nothing so we fall through to next statement
                    Serial.printf("Random jump not taken\n");
                }
            }
            break;

        default:
            Serial.printf("Ignoring unknown command 0x%02x%02x%02x\n", cmdBuf[0], cmdBuf[1], cmdBuf[2]);
            break;
    }
}

void
WHSign::startAnimation(char *filename) {
    if (animRunning) {
        animRunning = false;
    }

    if (animFile) {
        animFile.close();
    }

    if (!filename) {
        filename = "/anims/default.whb";
    }
    animFile = SPIFFS.open(filename, "r");

    if (!animFile) {
        Serial.printf("Could not open '%s'\n", filename);

        Dir d = SPIFFS.openDir("/");
        while(d.next()) {
            Serial.printf("%s %d\n", d.fileName().c_str(), d.fileSize());
        }

        return;
    }
    animRunning = true;
}
