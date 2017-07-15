#include <Arduino.h>
#include <ESPAsyncWebserver.h>
#include <Hash.h>
#include <functional>

#include "LEDArt.h"

class WebUI
{
public :
    WebUI(Nexus& nexus);

    void begin();

private:
    Nexus& nexus;
    AsyncWebServer ws = AsyncWebServer(80);
    AsyncWebSocket socket = AsyncWebSocket("/socket"); // access at ws://[esp ip]/socket

    void h_hello(AsyncWebServerRequest *req);
    void h_config_js(AsyncWebServerRequest *req);

    void h_404(AsyncWebServerRequest *req);

    void h_socket(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);


    void getAnimations(AsyncWebSocketClient * client);
    void getPalettes(AsyncWebSocketClient * client);
    void getState(AsyncWebSocketClient * client);

    void setAnimation(uint8_t *data, size_t len);
    void setUnit(uint8_t *data, size_t len);
    void setPalette(uint8_t *data, size_t len);

};