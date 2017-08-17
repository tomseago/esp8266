#pragma once

#include <Arduino.h>
#include <ESPAsyncWebserver.h>
#include <Hash.h>
#include <functional>

class WHSign;

class WebUI
{
    WHSign& sign;

public :
    WebUI(WHSign& sign);

    void begin();
    void broadcastState(uint16_t state);

private:
    AsyncWebServer ws = AsyncWebServer(80);
    AsyncWebSocket socket = AsyncWebSocket("/socket"); // access at ws://[esp ip]/socket

    void h_hello(AsyncWebServerRequest *req);
    void h_config_js(AsyncWebServerRequest *req);

    void h_404(AsyncWebServerRequest *req);

    void h_socket(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);


};