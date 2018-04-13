#include <Arduino.h>
#include <ESPAsyncWebserver.h>
#include <Hash.h>
#include <functional>

#include "LEDArt.h"


class WebUI
{
public :
    class StatusAnim : public LEDArtAnimation {
    public:
        StatusAnim(WebUI& parent);

        virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);

    private:
        WebUI& parent;
    };

    StatusAnim statusAnim;

    WebUI(Nexus& nexus);

    void begin();

private:
    Nexus& nexus;
    AsyncWebServer ws = AsyncWebServer(80);
    AsyncWebSocket socket = AsyncWebSocket("/socket"); // access at ws://[esp ip]/socket

    bool showingColorChooser = false;
    RgbColor chooserColor;

    void h_hello(AsyncWebServerRequest *req);
    void h_config_js(AsyncWebServerRequest *req);

    void h_404(AsyncWebServerRequest *req);

    void h_socket(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);


    void getAnimations(AsyncWebSocketClient * client);
    void getPalettes(AsyncWebSocketClient * client);
    void getState(AsyncWebSocketClient * client);

    void setAnimation(uint8_t *data, size_t len);
    void setGeometry(uint8_t *data, size_t len);
    void setPalette(uint8_t *data, size_t len);

    void setChooserColor(uint8_t *data, size_t len);
    void setNexusColor(bool isForeground, uint8_t *data, size_t len);

    void setBrightness(uint8_t *data, size_t len);
    void setDuration(uint8_t *data, size_t len);
    void setSpeedFactor(uint8_t *data, size_t len);
    void setReverse(uint8_t *data, size_t len);

};