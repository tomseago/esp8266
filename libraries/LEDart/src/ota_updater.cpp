#include "ota_updater.h"

#include "log.h"
#include "tagged_buffer.h"
#include "node_config.h"

#include <ESP8266httpUpdate.h>

const uint8_t TagVersion = 1;
const uint8_t TagURL = 2;

void
OtaUpdater::begin()
{
    udp.begin(OtaTriggerPort);
}

void
OtaUpdater::loop()
{
    if (udp.parsePacket())
    {
        // Got a packet!
        Log.printf("OTA: Got UDP notification packet\n");

        TaggedBuffer buf(udp.available());

        int r = udp.read((unsigned char *)buf.getData(), buf.getCapacity());
        buf.wrote(r);

        Log.printf("OTA: buf = ");
        buf.toLog();

        uint32_t version;
        char* szURL;

        if (!buf.find(TagVersion) || !buf.read(&version))
        {
            Log.printf("OTA: Couldn't read version. Ignoring.\n");
            udp.flush();
            return;            
        }

        if (!buf.find(TagURL) || !buf.read(&szURL))
        {
            Log.printf("OTA: Couldn't read url. Ignoring.\n");
            udp.flush();
            return;            
        }

        Log.printf("OTA: Learned about version=%d at %s\n", version, szURL);

        if (version != _version) 
        {
            Log.printf("OTA: Beginning OTA update\n");

            ESP8266HTTPUpdate update;

            update.rebootOnUpdate(true);

            String existing;
            existing += NodeConfig.nodeId();
            t_httpUpdate_return result = update.update(szURL, existing);
            switch(result)
            {
            case HTTP_UPDATE_OK:
                Log.printf("OTA: Success!! Should reboot in a moment..\n");
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Log.printf("OTA: Server said no updates are needed\n");
                break;

            case HTTP_UPDATE_FAILED:
                Log.printf("OTA: Update failed: %d %s\n", update.getLastError(), update.getLastErrorString().c_str());
                break;
            }
        }
        else
        {
            Log.printf("OTA: My internal version matches the notification. Ignoring it\n");
        }

        free(szURL);
        udp.flush();
    }
}
