#include "webui.h"
#include <FS.h>

#include <bstrlib.h>
#include "nexus.h"


// #include <StandardCplusplus.h>
// #include <string>

// using namespace std;
WebUI::StatusAnim::StatusAnim(WebUI& parent) : 
    LEDArtAnimation("WebUIStatus"),
    parent(parent)
{
    type = AnimationType_STATUS;
    maxDuration = 0;
}

void 
WebUI::StatusAnim::animate(LEDArtPiece& piece, AnimationParam p)
{
    if (parent.showingColorChooser) {
        piece.strip.ClearTo(parent.chooserColor);
    }
}


WebUI::WebUI(Nexus& nexus) :
    statusAnim(*this), nexus(nexus), chooserColor(255,128,0)
{

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


void 
WebUI::h_socket(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  if(type == WS_EVT_CONNECT){
    //client connected
    os_printf("ws[%s][%u] connect\n", server->url(), client->id());
    // client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    //client disconnected
    os_printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    //error was received from the other end
    os_printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    //pong message was received (in response to a ping request maybe)
    os_printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    //data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      os_printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
      if(info->opcode == WS_TEXT){
        data[len] = 0;
        os_printf("%s\n", (char*)data);
      } else {
        for(size_t i=0; i < info->len; i++){
          os_printf("%02x ", data[i]);
        }
        os_printf("\n");
      }

      ////////////////////
      // Handle the specific message

      if (len>=2) {
          switch (data[0]) {
            case 'G': // Get
                switch(data[1]) {
                    case 'A':
                        getAnimations(client);
                        break;

                    case 'P':
                        getPalettes(client);
                        break;

                    case 'X':
                        getState(client);
                        break;
                }
                break;

            case 'S':
                switch(data[1]) {
                    case 'A': // Animation
                        setAnimation(data, len);
                        break;

                    case 'U': // Unit
                        setUnit(data, len);
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
                }
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
                break;
          }
    }
      ////////////////////

      // if(info->opcode == WS_TEXT)
      //   client->text("I got your text message");
      // else
      //   client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          os_printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        os_printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      os_printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
      if(info->message_opcode == WS_TEXT){
        data[len] = 0;
        os_printf("%s\n", (char*)data);
      } else {
        for(size_t i=0; i < len; i++){
          os_printf("%02x ", data[i]);
        }
        os_printf("\n");
      }

      if((info->index + len) == info->len){
        os_printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          os_printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

void
WebUI::getAnimations(AsyncWebSocketClient * client)
{
    bstring s = bfromcstr("ANIMS:");


    for(uint8_t ix = 0; ix<nexus.numAnimations(); ix++)
    {
        char* sz = nexus.animName(ix);
        if (ix > 0)
        {
            bconchar(s, ';');
        }
        bcatcstr(s, sz);
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

    bformata(s,"%d;%d;",
        nexus.unitType,
        nexus.palette);

    char buf[8];
    bcatcstr(s,dtostrf(nexus.speedFactor, -7, 3, buf));

    bformata(s,";%02x%02x%02x;%s",
        nexus.foreground.R,
        nexus.foreground.G,
        nexus.foreground.B,

        nexus.currentAnim
        );

    client->text(bdata(s));
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
WebUI::setUnit(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    uint8_t t = atoi((const char *)&data[2]);

    if (t >= 6) {
        return;
    }

    nexus.unitType = t;
}

void
WebUI::setPalette(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    uint8_t max = (uint8_t)LEDArtAnimation::LEDPalette_LAST;
    uint8_t t = atoi((const char *)&data[2]);

    if (t >= max) {
        return;
    }

    nexus.palette = (LEDArtAnimation::LEDPaletteType)t;  
}

void
WebUI::setChooserColor(uint8_t *data, size_t len)
{
    HtmlColor color;
    uint8_t result = color.Parse<HtmlShortColorNames>((const char*)&data[1], len-1);

    chooserColor = RgbColor(color);
}

void
WebUI::setNexusColor(bool isForeground, uint8_t *data, size_t len)
{
    HtmlColor color;
    uint8_t result = color.Parse<HtmlShortColorNames>((const char*)&data[2], len-1);

    if (isForeground)
    {
        nexus.foreground = RgbColor(color);
    }
    else
    {
        nexus.background = RgbColor(color);
    }
}

void
WebUI::setBrightness(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    uint8_t t = atoi((const char *)&data[2]);

    nexus.maxBrightness = t;
}

void
WebUI::setDuration(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    uint32_t t = atoi((const char *)&data[2]);

    nexus.maxDuration = t * 1000;
}

void
WebUI::setSpeedFactor(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    float t = (float)atoi((const char *)&data[2]);

    nexus.speedFactor = t / 100.0f;
}

void
WebUI::setReverse(uint8_t *data, size_t len)
{
    if (len < 3) {
        return;
    }

    nexus.reverse = (data[2] == '+');
}