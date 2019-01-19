#include <Arduino.h>
#include <Print.h>

#include <ESPAsyncWebserver.h>
// #include <Hash.h>
#include <functional>
#include <bstrlib.h>

#include "LEDArt.h"


class WebUI : public Print, NexusListener
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

    WebUI(Nexus& nexus, LEDArtPiece& piece);

    void begin();
    void loop();

    virtual void nexusValueUpdate(NexusValueType which, uint32_t source);

    virtual void nexusUserGeometryRequest(char* szName, bool rotated, uint32_t source) { }

    // Can pass NULL as szName to ask for a random selection
    virtual void nexusUserAnimationRequest(char* szName, bool randomize, uint32_t source) { }

    ///////////////
    // The stream interface for writing logs
    void flush(void);
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    inline size_t write(unsigned long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t) n);
    }
    using Print::write; // pull in write(str) and write(buf, size) from Print

private:
    typedef struct ClientCtx {
        uint32_t id;
        bool wantsDeviceLogs;
        
        bool wantsStateJSON; // if they have requested it
        bool needsStateJSON; // if something has changed since we last updated them

        ClientCtx* pNext;

        ClientCtx(uint32_t id) : 
            id(id),
            wantsDeviceLogs(false),
            pNext(NULL)
            {}
            
    } ClientCtx;

    ClientCtx* pClientCtxs;

    void addClient(AsyncWebSocketClient* client);
    void removeClient(AsyncWebSocketClient* client);
    void removeClientForId(uint32_t id);
    ClientCtx* ctxForClient(AsyncWebSocketClient* client);


    Nexus& nexus;
    LEDArtPiece& piece;

    AsyncWebServer ws = AsyncWebServer(80);
    AsyncWebSocket socket = AsyncWebSocket("/socket"); // access at ws://[esp ip]/socket

    bool showingColorChooser = false;
    LAColor chooserColor;

    void h_hello(AsyncWebServerRequest *req);
    void h_config_js(AsyncWebServerRequest *req);

    void h_404(AsyncWebServerRequest *req);

    void h_socket(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

    void handleTextMessage(AsyncWebSocketClient* client, uint8_t* data, size_t len);

    void getGeometries(AsyncWebSocketClient * client);
    void getAnimations(AsyncWebSocketClient * client);
    void getPalettes(AsyncWebSocketClient * client);
    void getState(AsyncWebSocketClient * client);\
    void getStateJSON(AsyncWebSocketClient * client);

    void setGeometry(uint8_t *data, size_t len);
    void setAnimation(uint8_t *data, size_t len);
    void setPalette(uint8_t *data, size_t len);

    void setWantsLogs(AsyncWebSocketClient* client, uint8_t *data, size_t len);
    void setWantsStateJSON(AsyncWebSocketClient* client, uint8_t *data, size_t len);

    void setChooserColor(uint8_t *data, size_t len);
    void setNexusColor(bool isForeground, uint8_t *data, size_t len);

    void setBrightness(uint8_t *data, size_t len);
    void setDuration(uint8_t *data, size_t len);
    void setSpeedFactor(uint8_t *data, size_t len);
    void setReverse(uint8_t *data, size_t len);

    void setUpdateNeeded(AsyncWebSocketClient* client);
    bstring makeStateJSONMsg();
    void sendUpdates();
};