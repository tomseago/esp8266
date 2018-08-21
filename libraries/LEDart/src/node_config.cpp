#include "node_config.h"

#include <EEPROM.h>

#include <IPAddress.h>

#include "tagged_buffer.h"
#include "log.h"

// The global instance
NodeConfigClass NodeConfig;


const size_t EEPROM_Size = 512;

const uint8_t TagNodeId = 1;
const uint8_t TagBaseName = 2;
const uint8_t TagPassword = 3;

const uint8_t TagLaseHost = 4;

const uint8_t TagAPBaseIP = 5;
const uint8_t TagStaticMasterIP = 6;
const uint8_t TagStaticGateway = 7;

void
NodeConfigClass::begin(bool forceDefaults, uint8_t nodeId, char* baseName, char* password, uint32_t laseHost, uint32_t apBaseIP, uint32_t staticMasterIP, uint32_t staticGateway) 
{    
    _nodeId = nodeId;
    _szBaseName = baseName;
    _szPassword = password;
    _laseHost = laseHost;
    _apBaseIP = apBaseIP;
    _staticMasterIP = staticMasterIP;
    _staticGateway = staticGateway;

    if (forceDefaults)
    {
        Log.printf("CONFIG: ************* FORCED DEFAULTS **************\n");
        save();
        return;
    }

    // Read from EEPROM
    EEPROM.begin(EEPROM_Size);

    uint8_t* p = EEPROM.getDataPtr();
    if (p[0] != 'L' || p[1] != 'E' || p[2] != 'D' || p[3] != 'A')
    {
        Log.printf("CONFIG: No stored data\n");
        save();
    }
    else
    {
        // We've been here before!
        TaggedBuffer buf(p+4, EEPROM_Size-4);

        char* szTemp = NULL;
        uint32_t addr;

        if (buf.find(TagNodeId) && buf.read(&_nodeId))
        {
            Log.printf("CONFIG: Read nodeId = %d\n", _nodeId);
        }

        if (buf.find(TagBaseName) && buf.read(&szTemp))
        {
            _szBaseName = szTemp;
            Log.printf("CONFIG: Read basename = '%s'\n", _szBaseName);
        }

        if (buf.find(TagPassword) && buf.read(&szTemp))
        {
            _szPassword = szTemp;
            Log.printf("CONFIG: Read password = '%s'\n", _szPassword);
        }

        if (buf.find(TagLaseHost) && buf.read(&addr))
        {
            _laseHost = addr;
            Log.printf("CONFIG: Read laseHost = %s\n", IPAddress(_laseHost).toString().c_str());
        }

        if (buf.find(TagAPBaseIP) && buf.read(&addr))
        {
            _apBaseIP = addr;
            Log.printf("CONFIG: Read apBaseIP = %s\n", IPAddress(_apBaseIP).toString().c_str());
        }

        if (buf.find(TagStaticMasterIP) && buf.read(&addr))
        {
            _staticMasterIP = addr;
            Log.printf("CONFIG: Read staticMasterIP = %s\n", IPAddress(_staticMasterIP).toString().c_str());
        }

        if (buf.find(TagStaticGateway) && buf.read(&addr))
        {
            _staticGateway = addr;
            Log.printf("CONFIG: Read staticGateway = %s\n", IPAddress(_staticGateway).toString().c_str());
        }
    }

    EEPROM.end();
}


void
NodeConfigClass::save()
{
    EEPROM.begin(EEPROM_Size);

    uint8_t* p = EEPROM.getDataPtr();

    // our magic key
    *p++ = 'L';
    *p++ = 'E';
    *p++ = 'D';
    *p++ = 'A';

    TaggedBuffer buf(p, EEPROM_Size-4);

    buf.write(TagNodeId, _nodeId);
    buf.write(TagBaseName, _szBaseName);
    buf.write(TagPassword, _szPassword);
    buf.write(TagLaseHost, _laseHost);
    buf.write(TagAPBaseIP, _apBaseIP);
    buf.write(TagStaticMasterIP, _staticMasterIP);
    buf.write(TagStaticGateway, _staticGateway);
    
    buf.writeNull(); // So we don't keep reading forever

    EEPROM.commit();

    EEPROM.end();

    Log.printf("CONFIG: Saved to EEPROM\n");
}


