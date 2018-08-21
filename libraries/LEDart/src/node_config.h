#pragma once

#include <Arduino.h>

class NodeConfigClass {

    uint8_t _nodeId;

    char* _szBaseName;
    char* _szPassword;

    uint32_t _laseHost;    

    uint32_t _apBaseIP;
    uint32_t _staticMasterIP;
    uint32_t _staticGateway;

public:
    void begin(bool forceDefaults, uint8_t defNodeId, char* defBaseName, char* defPassword, uint32_t laseHost, uint32_t apBaseIP, uint32_t staticMasterIP, uint32_t staticGateway);

    uint8_t nodeId() { return _nodeId; }
    char* wifiBaseName() { return _szBaseName; }
    char* wifiPassword() { return _szPassword; }

    uint32_t laseHost() { return _laseHost; }

    uint32_t apBaseIP() { return _apBaseIP; }
    uint32_t staticMasterIP() { return _staticMasterIP; }
    uint32_t staticGateway() { return _staticGateway; }

    void save();

    void setBaseName(char* sz) { _szBaseName = sz; }
    void setPassword(char* sz) { _szPassword = sz; }

    void setLaseHost(uint32_t v) { _laseHost = v; }
    void setApBaseIP(uint32_t v) { _apBaseIP = v; }
    void setStaticMasterIP(uint32_t v) { _staticMasterIP = v; }
    void setStaticGateway(uint32_t v) { _staticGateway = v; }
};

extern NodeConfigClass NodeConfig;