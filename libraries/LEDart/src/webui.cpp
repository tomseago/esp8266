#include "webui.h"
#include <FS.h>
#include <string.h>

#include "nexus.h"
#include "log.h"
#include "butil.h"

#include <SPIFFS.h>

// For free memory
// extern "C" {
//#include "user_interface.h"
//#include <esp32/esp_system.h>
// }

// #include <StandardCplusplus.h>
// #include <string>

// using namespace std;
WebUI::StatusAnim::StatusAnim(WebUI& parent) : 
    LEDArtAnimation("WebUIStatus"),
    parent(parent)
{
    type = LEDAnimationType_STATUS;
    maxDuration = 0;
}

void 
WebUI::StatusAnim::animate(LEDArtPiece& piece, LEDAnimationParam p)
{
    if (parent.showingColorChooser) {
        piece.strip.ClearTo(parent.chooserColor);
    }
}


WebUI::WebUI(Nexus& nexus, LEDArtPiece& piece) :
    statusAnim(*this), nexus(nexus), piece(piece), chooserColor(255,128,0)
{
    nexus.addListener(this);
}

void
WebUI::addClient(AsyncWebSocketClient* client)
{
    if (!client) return;

    ClientCtx* ctx = new ClientCtx(client->id());
    if (!pClientCtxs)
    {
        pClientCtxs = ctx;
        return;
    }

    ClientCtx* pCur = pClientCtxs;
    while(pCur->pNext) pCur = pCur->pNext;

    pCur->pNext = ctx;
}

void
WebUI::removeClient(AsyncWebSocketClient* client)
{
    if (!client) return;

    removeClientForId(client->id());
}


void
WebUI::removeClientForId(uint32_t id)
{
    ClientCtx* pLast = NULL;
    ClientCtx* pCur = pClientCtxs;

    while(pCur)
    {
        if (id == pCur->id)
        {
            // time to kill it off
            if (!pLast)
            {
                // Only one item, so it must be at the head of the list
                pClientCtxs = pCur->pNext;
            }
            else
            {
                // There was someone before so rebase them to our future
                pLast->pNext = pCur->pNext;
            }

            // Don't modify last
            ClientCtx* pToKill = pCur;
            pCur = pCur->pNext;

            delete pToKill;
        }
        else
        {
            pLast = pCur;
            pCur = pCur->pNext;    
        }
    }
}

WebUI::ClientCtx* 
WebUI::ctxForClient(AsyncWebSocketClient* client)
{
    if (!client) return NULL;

    uint32_t id = client->id();
    ClientCtx* pCur = pClientCtxs;

    while(pCur)
    {
        if(pCur->id == id)
        {
            return pCur;
        }

        pCur = pCur->pNext;
    }

    // Nope!
    return NULL;
}

uint32_t lastPoke = 0;
uint32_t lastMemCheck = 0;
void
WebUI::loop()
{
    uint32_t now = millis();

    // if (now - lastPoke > 3000)
    // {
    //     // Log.print("poke\n");
    //     printf("poke!\n");
    //     lastPoke = now;
    // }    

    if (now - lastMemCheck > 10000) 
    {
        // uint32_t free = system_get_free_heap_size();
        uint32_t free = ESP.getFreeHeap();
        Serial.printf("WebUI: ------ Free memory %d\n", free);
        lastMemCheck = now;
    }


    sendUpdates();
}


void 
WebUI::begin(){

    // Need files Yo!
    SPIFFS.begin();

    // attach global request handler
    // classWebServer.on("/example", HTTP_ANY, handleRequest);

    socket.onEvent(std::bind(&WebUI::h_socket, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    // socket.onEvent(&WebUI::h_socket);
    ws.addHandler(&socket);

    // attach class request handler
    ws.on("/hello", HTTP_ANY, std::bind(&WebUI::h_hello, this, std::placeholders::_1));
    ws.on("/config.js", HTTP_ANY, std::bind(&WebUI::h_config_js, this, std::placeholders::_1));

    ws.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

    // Catch-All Handlers
    // Any request that can not find a Handler that canHandle it
    // ends in the callbacks below.
    ws.onNotFound(std::bind(&WebUI::h_404, this, std::placeholders::_1));

    ws.begin();

    // So that we can send these to our clients if they wants them
    Log.addPrint(this);
}



void 
WebUI::nexusValueUpdate(NexusValueType which, uint32_t source)
{
    if (source == (uint32_t)this) return;

    ClientCtx* pCur = pClientCtxs;
    if (!pCur) return;

    while(pCur)
    {
        if (pCur->wantsStateJSON)
        {
            pCur->needsStateJSON = true;
        }

        pCur = pCur->pNext;
    }
}



void 
WebUI::h_hello(AsyncWebServerRequest *req)
{
    AsyncResponseStream *response = req->beginResponseStream("text/html");

    response->printf("<!DOCTYPE html><html><head><title>Webpage at %s</title></head><body>", req->url().c_str());

    response->print("<h2>Hello ");
    response->print(req->client()->remoteIP());
    response->print("</h2>");

    response->print("<h3>General</h3>");
    response->print("<ul>");
    response->printf("<li>Version: HTTP/1.%u</li>", req->version());
    response->printf("<li>Method: %s</li>", req->methodToString());
    response->printf("<li>URL: %s</li>", req->url().c_str());
    response->printf("<li>Host: %s</li>", req->host().c_str());
    response->printf("<li>ContentType: %s</li>", req->contentType().c_str());
    response->printf("<li>ContentLength: %u</li>", req->contentLength());
    response->printf("<li>Multipart: %s</li>", req->multipart()?"true":"false");
    response->print("</ul>");

    response->print("<h3>Headers</h3>");
    response->print("<ul>");
    int headers = req->headers();
    for(int i=0;i<headers;i++){
      AsyncWebHeader* h = req->getHeader(i);
      response->printf("<li>%s: %s</li>", h->name().c_str(), h->value().c_str());
    }
    response->print("</ul>");

    response->print("<h3>Parameters</h3>");
    response->print("<ul>");
    int params = req->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = req->getParam(i);
      if(p->isFile()){
        response->printf("<li>FILE[%s]: %s, size: %u</li>", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        response->printf("<li>POST[%s]: %s</li>", p->name().c_str(), p->value().c_str());
      } else {
        response->printf("<li>GET[%s]: %s</li>", p->name().c_str(), p->value().c_str());
      }
    }
    response->print("</ul>");

    response->print("</body></html>");

    // Send it
    req->send(response);
}

void 
WebUI::h_config_js(AsyncWebServerRequest *req)
{
    AsyncResponseStream *response = req->beginResponseStream("application/javascript");

    response->printf("config={\"socketPath\":\"ws://%s/socket\"};", req->host().c_str());

    // Send it
    req->send(response);
}

void 
WebUI::h_404(AsyncWebServerRequest *req)
{
    AsyncResponseStream *response = req->beginResponseStream("text/html");

    response->setCode(404);

    response->print("<!DOCTYPE html><html><head><title>404 Not Found</title></head><body style='background: #000000; color: #ff0000;'>");
    response->print("<h1>404</h1>");
    response->print("Don't know what has happened, but");
    response->print("<center><flash>404 MOTHER FUCKER!!!!!!</flash></center>");
    response->print("</body></html>");

    // Send it
    req->send(response);  
}

// int h_socket_count = 1;

void 
WebUI::h_socket(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{    
    // Serial.printf("h_socket(,,%d,%x,%x,%d)\n", type, arg, data, len);

    // if (h_socket_count==0) {
    //     Serial.printf("**** done\n");
    //     return;
    // }
    // h_socket_count--;

    if(type == WS_EVT_CONNECT){
        //client connected
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        // client->ping();

        addClient(client);

    } else if(type == WS_EVT_DISCONNECT){
        //client disconnected
        Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
        removeClient(client);

    } else if(type == WS_EVT_ERROR){
        //error was received from the other end
        Serial.printf("ws[%s][%u] error(%u):\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
        // Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);

    } else if(type == WS_EVT_PONG){
        //pong message was received (in response to a ping request maybe)
        // Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
        Serial.printf("ws[%s][%u] pong[%u]\n", server->url(), client->id(), len);

    } else if(type == WS_EVT_DATA) {
        //data packet
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        // Serial.printf("ws[%s][%u] data final=%d index=%d iLen=%d len=%d\n",
        //     server->url(), client->id(),
        //     info->final, info->index, info->len, len
        // );

        if(info->final && info->index == 0 && info->len == len){
            //the whole message is in a single frame and we got all of it's data

            // Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
            //   for(size_t i=0; i < info->len; i++){
            //     Serial.printf("%02x ", data[i]);
            //   }
            //   Serial.printf("\n");
            // }

            if (info->opcode != WS_TEXT) {
                // TODO: Handle binary??
                Serial.printf("ws do not handle binary messages\n");
                return;
            }

            // It is text

            if (len<2) {
                Serial.printf("ws message was too short. Needs to be 2 bytes\n");
                return;
            }

            ////////////////////
            // Handle the specific message

            // Life is WAY better if the message is a null terminated string
            uint8_t* szTemp = (uint8_t*)strndup((char*)data, len);
            if (!szTemp) {
                Serial.printf("ws OOM couldn't create szTemp to handle the message\n");
                return;
            }
            Serial.printf("ws[%s][%u] handleTextMessage(%s)\n", 
                server->url(), client->id(),
                szTemp
            );

            handleTextMessage(client, szTemp, len);
            free(szTemp);

        
        } else {
            Serial.printf("\nXXXX Not handling segmented messages\n");
            // //message is comprised of multiple frames or the frame is split into multiple packets
            // if (info->index == 0) {
            //     if(info->num == 0)
            //         Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
            //     Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            // }

            // Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
            // if(info->message_opcode == WS_TEXT) {
            //     data[len] = 0;
            //     Serial.printf("%s\n", (char*)data);
            // } else {
            //     for(size_t i=0; i < len; i++){
            //         Serial.printf("%02x ", data[i]);
            //     }
            //     Serial.printf("\n");
            // }

            // if((info->index + len) == info->len) {
            //     Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
            //     if(info->final) {
            //         Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
            //         if(info->message_opcode == WS_TEXT)
            //             client->text("I got your text message");
            //         else
            //             client->binary("I got your binary message");
            //     }
            // }
        }
    }
}

void
WebUI::handleTextMessage(AsyncWebSocketClient* client, uint8_t* data, size_t len)
{
    switch (data[0]) {
    case 'G': // Get
        switch(data[1]) {
            case 'G':
                getGeometries(client);
                break;

            case 'A':
                getAnimations(client);
                break;

            case 'P':
                getPalettes(client);
                break;

            case 'X':
                getState(client);
                break;

            case 'J':
                getStateJSON(client);
                break;
        }
        break;

    case 'S':
        switch(data[1]) {
            case 'G': // Geometry
                setGeometry(data, len);
                break;

            case 'A': // Animation
                setAnimation(data, len);
                break;

            case 'P': // Palette
                setPalette(data, len);
                break;

            case 'B': // Brightness
                setBrightness(data, len);
                break;

            case 'D': // Max Duration (in seconds)
                setDuration(data, len);
                break;

            case 'S': // Speed factor as integer in cents
                setSpeedFactor(data, len);
                break;

            case 'R': // Reverse state
                setReverse(data, len);
                break;

            case 'L':
                setWantsLogs(client, data, len);
                break;

            case 'J':
                setWantsStateJSON(client, data, len);
                break;
        }
        setUpdateNeeded(client);
        break;

    case 'C':
        switch(data[1]) {
            case '+': // On
                showingColorChooser = true;
                break;

            case '-': // Off
                showingColorChooser = false;
                break;

            case '#':
                setChooserColor(data, len);
                break;

            case 'F': // Foreground
                setNexusColor(true, data, len);
                break;

            case 'B': // Background
                setNexusColor(false, data, len);
                break;
        }
        setUpdateNeeded(client);
        break;
    }    
}

void
WebUI::getGeometries(AsyncWebSocketClient * client)
{
    bstring s = bfromcstr("GEOMS:");

    char* szName = NULL;
    bool canRotate = false;
    void* cursor = piece.enumerateGeometries(NULL, &szName, &canRotate);
    while(cursor)
    {
        if (szName)
        {
            bconchar(s, canRotate ? '+' : '-');
            bcatcstr(s, szName);
            bconchar(s, ';');
        }

        cursor = piece.enumerateGeometries(cursor, &szName, &canRotate);
    }

    if (szName)
    {
        bconchar(s, canRotate ? '+' : '-');
        bcatcstr(s, szName);
        bconchar(s, ';');
    }

    client->text(bdata(s));
}


void
WebUI::getAnimations(AsyncWebSocketClient * client)
{
    bstring s = bfromcstr("ANIMS:");

    char* szName = NULL;
    void* cursor = piece.enumerateAnimations(NULL, &szName);
    while(cursor)
    {
        if (szName)
        {
            bcatcstr(s, szName);
            bconchar(s, ';');
        }

        cursor = piece.enumerateAnimations(cursor, &szName);
    }

    if (szName)
    {
        bcatcstr(s, szName);
        bconchar(s, ';');
    }

    client->text(bdata(s));
}

void
addPalette(bstring s, LEDArtAnimation::LEDPaletteType type) 
{
    for(uint8_t i = 0; i<LEDArtAnimation::paletteSizes[type]; i++) 
    {
        if (i > 0) {
            bconchar(s, ',');
        }
        bformata(s, "%02x%02x%02x", 
            LEDArtAnimation::paletteColors[type][i].R,
            LEDArtAnimation::paletteColors[type][i].G,
            LEDArtAnimation::paletteColors[type][i].B);

    }
    bconchar(s, ';');
}

void
WebUI::getPalettes(AsyncWebSocketClient * client)
{
    bstring s = bfromcstr("PAL:");


    LEDArtAnimation::LEDPaletteType type = (LEDArtAnimation::LEDPaletteType)0;
    while(type != LEDArtAnimation::LEDPalette_LAST)
    {
        addPalette(s, type);

        // C++ is stupid sometimes...
        type = (LEDArtAnimation::LEDPaletteType)((uint8_t)type + 1);
    }



    client->text(bdata(s));
}

void
WebUI::getState(AsyncWebSocketClient * client)
{
    bstring s = bfromcstr("STATE:");

    bformata(s,"%d;",
        nexus.palette);

    char buf[8];
    bcatcstr(s,dtostrf(nexus.speedFactor, -7, 3, buf));

    bformata(s,";%02x%02x%02x;%s",
        nexus.foreground.R,
        nexus.foreground.G,
        nexus.foreground.B,

        nexus.getCurrentAnimName()
        );

    client->text(bdata(s));
}


bstring
WebUI::makeStateJSONMsg()
{
    bstring s = bfromcstr("STATEJSON:{");
    char buf[8];

    bformata(s,"\"palette\":%d",nexus.palette);

    bcatcstr(s, ",\"speedFactor\":");
    bcatcstr(s,dtostrf(nexus.speedFactor, -7, 3, buf));

    bcatcstr(s, ",\"maxDuration\":");
    bcatcstr(s,dtostrf(nexus.maxDuration, -7, 3, buf));

    bcatcstr(s, ",\"maxBrightness\":");
    bcatcstr(s,dtostrf(nexus.maxBrightness, -7, 3, buf));

    bformata(s,",\"foreground\":[%d,%d,%d]",
        nexus.foreground.R,
        nexus.foreground.G,
        nexus.foreground.B
        );
    bformata(s,",\"background\":[%d,%d,%d]",
        nexus.background.R,
        nexus.background.G,
        nexus.background.B
        );
    bformata(s,",\"reverse\":%s", nexus.reverse ? "true" : "false");
    bformata(s,",\"geomRotated\":%s", nexus.isGeomRotated() ? "true" : "false");
    bformata(s,",\"geomName\":\"%s\"", nexus.getCurrentGeomName());
    bformata(s,",\"animName\":\"%s\"", nexus.getCurrentAnimName());

    bcatcstr(s,"}");

    return s;    
}

void
WebUI::getStateJSON(AsyncWebSocketClient * client)
{
    bstring s = makeStateJSONMsg();

    client->text(bdata(s));

    bdestroy(s);
}

void
WebUI::setAnimation(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    bool randomize = false;
    if (data[2]=='+') {
        randomize = true;
    }

    char* szName = (len==3) ? NULL : (char*)&data[3];
    nexus.sendUserAnimationRequest(szName, randomize, (uint32_t)this);
}

void
WebUI::setGeometry(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    bool rotated = false;
    if (data[2]=='+') {
        rotated = true;
    }

    char* szName = (len==3) ? NULL : (char*)&data[3];
    nexus.sendUserGeometryRequest(szName, rotated, (uint32_t)this);
}

// void
// WebUI::setUnit(uint8_t *data, size_t len)
// {
//     if (len < 3) {
//         return;
//     }

//     uint8_t t = atoi((const char *)&data[2]);

//     if (t >= 6) {
//         return;
//     }

//     nexus.unitType = (LEDArtAnimation::LEDUnitType)t;
//     nexus.checkUnitType();
// }

void
WebUI::setPalette(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    uint8_t max = (uint8_t)LEDArtAnimation::LEDPalette_LAST;
    int t = buf_toi(data+2, len-2);

    if (t >= max) {
        return;
    }

    nexus.palette = (LEDArtAnimation::LEDPaletteType)t;
    nexus.sendValueUpdate(NexusListener::Palette, (uint32_t)this);
}

void
WebUI::setWantsLogs(AsyncWebSocketClient* client, uint8_t *data, size_t len)
{
    if (!client) return;

    if (len < 3) return;

    bool wantsIt = (data[2] == '+');

    ClientCtx* ctx = ctxForClient(client);
    if (ctx)
    {
        ctx->wantsDeviceLogs = wantsIt;
    }
}


void
WebUI::setWantsStateJSON(AsyncWebSocketClient* client, uint8_t *data, size_t len)
{
    if (!client) return;

    if (len < 3) return;

    bool wantsIt = (data[2] == '+');

    ClientCtx* ctx = ctxForClient(client);
    if (ctx)
    {
        ctx->wantsStateJSON = wantsIt;
    }
}

void
WebUI::setChooserColor(uint8_t *data, size_t len)
{
    HtmlColor color;
    uint8_t result = color.Parse<HtmlShortColorNames>((const char*)&data[1], len-1);

    chooserColor = LAColor(color);
}

void
WebUI::setNexusColor(bool isForeground, uint8_t *data, size_t len)
{
    HtmlColor color;
    uint8_t result = color.Parse<HtmlShortColorNames>((const char*)&data[2], len-1);

    if (isForeground)
    {
        nexus.foreground = LAColor(color);
        nexus.sendValueUpdate(NexusListener::Foreground, (uint32_t)this);
    }
    else
    {
        nexus.background = LAColor(color);
        nexus.sendValueUpdate(NexusListener::Background, (uint32_t)this);
    }
}

void
WebUI::setBrightness(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    // Log.printf("setBrightness(%s)\n", &data[2]);
    // bstring s = blk2bstr(data+2, len-2);
    // uint8_t t = atoi(bdata(s));
    uint8_t t = buf_toi(data + 2, len-2);
    nexus.maxBrightness = t;
    nexus.sendValueUpdate(NexusListener::MaxBrightness, (uint32_t)this);
}

void
WebUI::setDuration(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    uint32_t t = buf_toi(data + 2, len-2);

    nexus.maxDuration = t * 1000;
    nexus.sendValueUpdate(NexusListener::MaxDuration, (uint32_t)this);
}

void
WebUI::setSpeedFactor(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    float t = (float)buf_toi(data + 2, len-2);

    nexus.speedFactor = t / 100.0f;
    nexus.sendValueUpdate(NexusListener::SpeedFactor, (uint32_t)this);
}

void
WebUI::setReverse(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    nexus.reverse = (data[2] == '+');
    nexus.sendValueUpdate(NexusListener::Reverse, (uint32_t)this);
}


///////////////////////
void 
WebUI::setUpdateNeeded(AsyncWebSocketClient* client)
{
    ClientCtx* ctx = ctxForClient(client);
    if (ctx && ctx->wantsStateJSON)
    {
        ctx->needsStateJSON = true;
    }
}

void
WebUI::sendUpdates()
{
    ClientCtx* pCur = pClientCtxs;
    if (!pCur) return;

    bool haveMsg = false;
    bstring s;

    while(pCur)
    {
        if (pCur->wantsStateJSON && pCur->needsStateJSON)
        {
            if (!haveMsg) {
                s = makeStateJSONMsg();
                haveMsg = true;
            }

            AsyncWebSocketClient* client = socket.client(pCur->id);
            // Log.printf("DL For %d got %d\n", pCur->id, client);
            if (!client)
            {
                // Error of some type???
            }
            else
            {
                client->text(bdata(s));
            }
        }
        pCur->needsStateJSON = false;

        pCur = pCur->pNext;
    }

    if (haveMsg) 
    {
        bdestroy(s);
    }
    return; 
}

///////////////////////
// Stream interface for logging

void
WebUI::flush(void)
{
}

size_t
WebUI::write(uint8_t c)
{
    // Should buffer these, but we will not for right now...
    return write(&c, 1);
}

size_t
WebUI::write(const uint8_t *buffer, size_t size)
{
    if (!buffer || !size) return 0;

    ClientCtx* pCur = pClientCtxs;
    if (!pCur) return 0;

    bstring s = bfromcstr("L:");
    bcatblk(s, buffer, size);

    // Log.printf("DL>%s<\n", bdata(s));

    while(pCur)
    {
        if (pCur->wantsDeviceLogs)
        {
            AsyncWebSocketClient* client = socket.client(pCur->id);
            // Log.printf("DL For %d got %d\n", pCur->id, client);
            if (!client)
            {
                // Error of some type???
            }
            else
            {
                client->text(bdata(s));
            }
        }

        pCur = pCur->pNext;
    }

    bdestroy(s);

    return size;
}
