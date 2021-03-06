#include <stdio.h>
#include <functional>

#include "msg_tube.h"
#include "log.h"

#include <ESPAsyncTCPbuffer.h>


const uint32_t STA_Restart_Delay = 20000;
const uint32_t STA_Connected_Recheck_Delay = 30000;
const uint32_t STA_Attempt_Window = 20000;

const uint32_t STA_Attempt_Check_Delay = 2000;

class MTClientConnection {    

    MTMessage msg;

public:
    AsyncTCPbuffer* tcpBuffer = NULL;
    MTClientConnection* next = NULL;

    bool wantsFlush;

    uint8_t addr;

    MTClientConnection();
    bool begin(AsyncClient* clientIn);

    void sendAddr(uint8_t nodeId);
    void readAddr(bool ok, void* ret);

    void messageDone(bool ok);

    bool handleDisconnect(AsyncTCPbuffer* obj);

    void flushIfNeeded();
};

// Clean up the AsyncClient?????

MTClientConnection::MTClientConnection() :
    msg(512),
    addr(255)
{
}

bool
MTClientConnection::begin(AsyncClient* clientIn)
{
    if (!clientIn) return false;

    tcpBuffer = new AsyncTCPbuffer(clientIn);

    if (!tcpBuffer)
    {
        Log.println("Unable to create AsyncTCPBuffer\n");
        return false;
    }

    tcpBuffer->onDisconnect(std::bind(&MTClientConnection::handleDisconnect, this, std::placeholders::_1));

    // Start with reading our connection data
    tcpBuffer->readBytes((uint8_t*)&addr, sizeof(addr), std::bind(&MTClientConnection::readAddr, this, std::placeholders::_1, std::placeholders::_2));

    return true;
}

void
MTClientConnection::sendAddr(uint8_t nodeId)
{
    if (!tcpBuffer) return;

    if (!tcpBuffer->write(nodeId))
    {
        Log.printf("MT: ERROR failed to send my nodeId\n");
        tcpBuffer->close();
    }
    else
    {
        // To make sure it goes right away...
        tcpBuffer->flush();
    }
}

void
MTClientConnection::readAddr(bool ok, void* ret)
{
    if (!ok) {
        Log.printf("MT: ERROR: Trying to read addr\n");
        tcpBuffer->close();
        return;
    }

    Log.printf("MT: %s has address %d\n", tcpBuffer->remoteIP().toString().c_str(), addr);

    // Let's read a message!
    msg.readFrom(this);
}

void
MTClientConnection::messageDone(bool ok)
{
    if (!tcpBuffer) return;

    if (!ok)
    {
        // Aack!
        Log.printf("buffer close\n");
        tcpBuffer->close();
        return;
    }

    // Read the message, pass it up the chain so that it can be handled inside
    // the main server or routed or whatever
    msgTube.receivedMessage(this, &msg);

    // Read next message 
    msg.readFrom(this);
}

bool
MTClientConnection::handleDisconnect(AsyncTCPbuffer* obj)
{
    if (!tcpBuffer) return true;

    Log.printf("MT: disconnect from %v\n", tcpBuffer->remoteIP().toString().c_str());

    tcpBuffer = NULL;
    msgTube.connectionClosed(this);
    return true;
}

void
MTClientConnection::flushIfNeeded()
{
    if (!wantsFlush) return;
    wantsFlush = false;

    if (!tcpBuffer) return;
    tcpBuffer->flush();
}

/////////////////////////////////////////////////////////////////////////////////////


MTMessage::MTMessage(uint16_t capacity) :
    id(0),
    src(0),
    dest(0),
    type(0),
    capacity(capacity),
    length(0)
{
    if (capacity > 0)
    {
        data = (uint8_t*)malloc(capacity);
    }
    else
    {
        data = NULL;
    }
}

MTMessage::~MTMessage()
{
    if (data)
    {
        free(data);        
    }
}

void
MTMessage::catData(uint8_t* input, uint16_t inputLen)
{
    uint16_t toCopy = inputLen;
    if (capacity - length < inputLen) 
    {
        toCopy = capacity - length;
    }

    if (toCopy==0) return;

    memcpy(data + length, input, toCopy);

    length += toCopy;
}

void
MTMessage::setData(uint8_t* input, uint16_t inputLen)
{
    uint16_t toCopy = inputLen;
    if (toCopy > capacity) 
    {
        toCopy = capacity;
    }

    if (toCopy == 0) 
    {
        length = 0;
        return;
    }
    memcpy(data, input, toCopy);

    length = toCopy;

}

void
MTMessage::readFrom(MTClientConnection* conn)
{
    // Start at the id
    this->conn = conn;
    conn->tcpBuffer->readBytes((uint8_t*)&id, sizeof(id), std::bind(&MTMessage::readIdDone, this, std::placeholders::_1, std::placeholders::_2));
}

void 
MTMessage::readIdDone(bool ok, void * ret)
{
    if (!ok) {
        Log.printf("MT: ERROR: Trying to read message id\n");
        conn->messageDone(false);
        return;
    }

    // Log.printf("MT: Read: id = %d\n", id);

    conn->tcpBuffer->readBytes((uint8_t*)&src, sizeof(src), std::bind(&MTMessage::readSrcDone, this, std::placeholders::_1, std::placeholders::_2));    
}

void 
MTMessage::readSrcDone(bool ok, void * ret)
{
    if (!ok) {
        Log.printf("MT: ERROR: Trying to read message source\n");
        conn->messageDone(false);
        return;
    }

    // Log.printf("MT: Read: source = %d\n", src);

    conn->tcpBuffer->readBytes((uint8_t*)&dest, sizeof(dest), std::bind(&MTMessage::readDestDone, this, std::placeholders::_1, std::placeholders::_2));    
}

void 
MTMessage::readDestDone(bool ok, void * ret)
{
    if (!ok) {
        Log.printf("MT: ERROR: Trying to read message dest\n");
        conn->messageDone(false);
        return;
    }

    // Log.printf("MT: Read: dest = %d\n", dest);

    conn->tcpBuffer->readBytes((uint8_t*)&type, sizeof(type), std::bind(&MTMessage::readTypeDone, this, std::placeholders::_1, std::placeholders::_2));    
}

void 
MTMessage::readTypeDone(bool ok, void * ret)
{
    if (!ok) {
        Log.printf("MT: ERROR: Trying to read message type\n");
        conn->messageDone(false);
        return;
    }

    // Log.printf("MT: Read: type = %d\n", type);

    conn->tcpBuffer->readBytes((uint8_t*)&length, sizeof(length), std::bind(&MTMessage::readDataLenDone, this, std::placeholders::_1, std::placeholders::_2));    
}

void 
MTMessage::readDataLenDone(bool ok, void * ret)
{
    if (!ok) {
        Log.printf("MT: ERROR: Trying to read message data length\n");
        conn->messageDone(false);
        return;
    }

    // Log.printf("MT: Read: length = %d\n", length);

    // Make sure this is sane
    if (length > capacity)
    {
        Log.printf("MT: ERROR: Maximum message length is %d\n", capacity);
        conn->messageDone(false);
        return;
    }

    if (length) 
    {
        // Length is okay, so read the data now
        conn->tcpBuffer->readBytes(data, length, std::bind(&MTMessage::readDataDone, this, std::placeholders::_1, std::placeholders::_2));            
    }
    else
    {
        // No data to ready, we are done
        conn->messageDone(true);
    }


}

void
MTMessage::readDataDone(bool ok, void * ret)
{
    if (!ok) {
        Log.printf("MT: ERROR: Trying to read message data\n");
        conn->messageDone(false);
        return;
    }

    // All done - yay!!!
    conn->messageDone(true);
}

bool
MTMessage::readFromUDP(WiFiUDP* pUDP)
{
    if (!pUDP) return false;

    int avail = pUDP->available();
    Log.printf("MT: Read from UDP avail=%d\n", avail);

    if (avail < sizeof(id))
    {
        Log.printf("MT: ERROR: UDP message didn't have enough bytes for id\n");
        return false;
    }
    pUDP->read((unsigned char*)&id, sizeof(id));
    avail -= sizeof(id);

    if (avail < sizeof(src))
    {
        Log.printf("MT: ERROR: UDP message didn't have enough bytes for src\n");
        return false;
    }
    pUDP->read((unsigned char*)&src, sizeof(src));
    avail -= sizeof(src);

    if (avail < sizeof(dest))
    {
        Log.printf("MT: ERROR: UDP message didn't have enough bytes for dest\n");
        return false;
    }
    pUDP->read((unsigned char*)&dest, sizeof(dest));
    avail -= sizeof(dest);

    if (avail < sizeof(type))
    {
        Log.printf("MT: ERROR: UDP message didn't have enough bytes for type\n");
        return false;
    }
    pUDP->read((unsigned char*)&type, sizeof(type));
    avail -= sizeof(type);

    if (avail < sizeof(length))
    {
        Log.printf("MT: ERROR: UDP message didn't have enough bytes for id\n");
        return false;
    }
    pUDP->read((unsigned char*)&length, sizeof(length));
    avail -= sizeof(length);

    if (length > capacity)
    {
        Log.printf("MT: ERROR: UDP message has data length %d but capacity is %d\n", length, capacity);
        return false;
    }

    if (length > avail)
    {
        Log.printf("MT: ERROR: UDP data length=%d but avail is only=%d\n", length, avail);
        return false;
    }
    pUDP->read((unsigned char*)data, length);

    return true;
}

////

// Send the message. If something goes bad presumable the connection closes???
void
MTMessage::sendTo(MTClientConnection* conn)
{
    if (!conn) return;

    Log.printf("MT: Sending %d(%d: %d->%d) to %d\n", id, type, src, dest, conn->addr);

    conn->tcpBuffer->write((uint8_t*)&id, sizeof(id));
    conn->tcpBuffer->write((uint8_t*)&src, sizeof(src));
    conn->tcpBuffer->write((uint8_t*)&dest, sizeof(dest));
    conn->tcpBuffer->write((uint8_t*)&type, sizeof(type));
    conn->tcpBuffer->write((uint8_t*)&length, sizeof(length));

    if (length > 0)
    {
        conn->tcpBuffer->write(data, length);
    }

    // Can not call flush() from here, probably because we might be responding
    // to a message, meaning we are on the interrupt task thing, and somehow flush
    // conflicts with that.
    conn->wantsFlush = true;
}

void
MTMessage::writeToUDP(WiFiUDP* pUDP)
{
    if (!pUDP) return;

    Log.printf("MT: Writing %d(%d: %d->%d) to UDP\n", id, type, src, dest);

    pUDP->write((uint8_t*)&id, sizeof(id));
    pUDP->write((uint8_t*)&src, sizeof(src));
    pUDP->write((uint8_t*)&dest, sizeof(dest));
    pUDP->write((uint8_t*)&type, sizeof(type));
    pUDP->write((uint8_t*)&length, sizeof(length));

    if (length > 0)
    {
        pUDP->write(data, length);
    }
}

/////////////////////////////////////////////////////////////////////////////////////

MsgTube::MsgTube() :
    server(MTServerPort),
    udpMsg(1024)
{

}

void
MsgTube::enableStatic()
{
    isStaticMode = true;
}

void
MsgTube::enableUDP(uint8_t maxNodes)
{
    isStaticMode = true;
    useUDP = true;

    maxNodes++;

    pUDP = new WiFiUDP();
    this->maxNodes = maxNodes;
    learnedIPs = (IPAddress*)malloc(maxNodes * sizeof(IPAddress));
    // Start with everyone being broadcast is the way we reach them
    // This is some legit ignoring C++ decoration of memory...
    memset(learnedIPs, 0xff, maxNodes * sizeof(IPAddress));

    // But for master we know the IP yeah? BUT, let us learn it...
}

void
MsgTube::begin()
{
    // Don't worry about storing things in flash
    WiFi.persistent(false);
    // Don't start wifi on boot
    WiFi.setAutoConnect(false);
    // Don't try reconnecting because we will handle that case
    WiFi.setAutoReconnect(false);

    //------ Configure the AP side of things
    if (!isStaticMode)
    {
        WiFi.mode(WIFI_AP_STA);
        char szName[strlen(NodeConfig.wifiBaseName()) + 3];
        sprintf(szName, "%s%d", NodeConfig.wifiBaseName(), NodeConfig.nodeId());

        Log.printf("MT: Starting soft AP %s %s\n", szName, NodeConfig.wifiPassword());
        WiFi.softAP(szName, NodeConfig.wifiPassword());


        IPAddress localIp(NodeConfig.apBaseIP());
        IPAddress gateway(NodeConfig.apBaseIP());
        IPAddress subnet(255, 255, 255, 0);

        localIp[2] = NodeConfig.nodeId() + 10;
        // localIp[3] = nodeId + 10;
        gateway = localIp;


        Log.printf("MT: Calling softAPConfig(%s, %s, %s)\n", localIp.toString().c_str(), gateway.toString().c_str(), subnet.toString().c_str());
        WiFi.softAPConfig(localIp, gateway, subnet);
    }
    else
    {
        WiFi.mode(WIFI_STA);        
    }

    //------- Setup our server for receiving connections
    server.onClient(std::bind(&MsgTube::sConnect, this, std::placeholders::_1, std::placeholders::_2), NULL);
    server.begin();

    Log.printf("MT: Started server on port %d\n", MTServerPort);

    //------- Start trying to connect to upstream peers
}

void
MsgTube::loop()
{
    checkSTA();

    // Flush any connections that need it
    MTClientConnection* cursor = clientConns;
    while(cursor)
    {
        MTClientConnection* next = cursor->next;

        cursor->flushIfNeeded();

        cursor = next;
    }

    receiveUDP();
}


void
MsgTube::addListener(MTListener* listener)
{
    if (!listener) return;

    listener->nextMTListener = listeners;
    listeners = listener;
}

// Use this to create all new messages. The returned message object is owned
// by the MsgTube.
MTMessage* 
MsgTube::newMessage(uint16_t capacity)
{
    // TODO: Smarter recycling of messages from a pool maybe???
    MTMessage* msg = new MTMessage(capacity);
    if (msg)
    {
        msg->src = NodeConfig.nodeId();
    }
    return msg;
}

// Send a previously created message. After this is called the message object
// is under the control of the tube and should not be modified.
void
MsgTube::sendMessage(MTMessage* msg)
{
    if (!msg) return;

    uint8_t nodeId = NodeConfig.nodeId();

    if (msg->dest == nodeId)
    {
        // Refuse to send messages to ourself
        return;
    }

    if (useUDP)
    {
        // Perhapse UDP?
        sendUDP(msg);
    }
    else
    {
        bool sendAbove = false;
        bool sendBelow = false;

        if (msg->src == nodeId)
        {
            // We are sending it, so use the destination to know above or below.
            if (msg->dest == MTBroadcastAddr)
            {
                sendAbove = true;
                sendBelow = true;
            }
            else
            {
                sendAbove = msg->dest < nodeId;
                sendBelow = msg->dest > nodeId;
            }
        }
        else
        {
            // Look at the source and send it the opposite direction
            sendAbove = msg->src > nodeId;
            sendBelow = msg->src < nodeId;
        }    

        MTClientConnection* cursor = clientConns;

        while(cursor)
        {
            // Because sending the message might do a failure that deletes it maybe??
            MTClientConnection* next = cursor->next;

            if (sendAbove && cursor->addr < nodeId)
            {
                msg->sendTo(cursor);
            }
            if (sendBelow && cursor->addr > nodeId)
            {
                msg->sendTo(cursor);
            }

            // Loop!
            cursor = next;
        }
    }

    Log.printf("MT: sendMessage() complete\n");
}


//////////////////
void
MsgTube::sConnect(void* context, AsyncClient* client)
{
    if (!client) return;

    Log.printf("MT: Connect from %s on %s\n", client->remoteIP().toString().c_str(), client->localIP().toString().c_str());

    MTClientConnection* conn = new MTClientConnection();
    if (!conn->begin(client)) 
    {
        delete conn;
        return;
    }

    // Save this new connection
    addConnection(conn);
}

void
MsgTube::conError(void* context, AsyncClient* client, int8_t error)
{
    if (!client) return;

    Log.printf("MT: Error on connect %d\n", error);
}

void
MsgTube::addConnection(MTClientConnection* conn)
{
    if (!conn) return;

    // Put most recent at the head
    conn->next = clientConns;
    clientConns = conn;

    // dumpClientConns();
    conn->sendAddr(NodeConfig.nodeId());    
}

void
MsgTube::connectionClosed(MTClientConnection* conn)
{
    Log.printf("MT: client connection closed %p\n", conn);


    // dumpClientConns();

    // Remove it from the list of connections
    MTClientConnection* prev = NULL;
    MTClientConnection* cursor = clientConns;
    while(cursor)
    {
        if (cursor == conn)
        {
            if (prev) 
            {
                prev->next = cursor->next;
            }
            else
            {
                clientConns = cursor->next;
            }
        }
        else
        {
            prev = cursor;
        }
        cursor = cursor->next;
    }

    // Log.printf("MT: Delete done\n");

    // dumpClientConns();

    // Like really, nuke it...
    delete conn;
}

void
MsgTube::receivedMessage(MTClientConnection* conn, MTMessage* msg)
{
    Log.printf("MT: Received a complete message\n");

    if (msg->dest == NodeConfig.nodeId() || msg->dest == MTBroadcastAddr)
    {
        // It's for me!
        handleMessage(msg);

        if (msg->dest != MTBroadcastAddr) 
        {
            // It was for me, so eat it
            return;
        }
    }

    // Now on to our regularly scheduled above or below routing
    sendMessage(msg);
}

void
MsgTube::dumpClientConns()
{
    MTClientConnection* cursor = clientConns;
    if (!cursor)
    {
        Log.printf("MT: ClientCons -- empty\n");
        return;
    }

    while(cursor) 
    {
        Log.printf("MT: ClientCons %p, next->%p\n", cursor, cursor->next);
        cursor = cursor->next;
    }
}

///////

void
MsgTube::checkSTA()
{
    uint32_t now = millis();
    if (now < nextSTACheck)
    {
        return;
    }

    //Log.printf("MT: STA check in state %d\n", staStatus);

    switch(staStatus) {
    case Nothing:
        if (isStaticMode) {
            startStaticConnect();
        } else {
            startSTAConnect(false);
        }
        break;

    case TryingPrimary:
        checkSTAAttempt(false);
        break;

    case TryingSecondary:
        checkSTAAttempt(true);
        break;

    case ConnectedPrimary:
        checkSTAStillOk(false);
        break;

    case ConnectedSecondary:
        checkSTAStillOk(true);
        break;

    case Disabled:
        // Do nothing
        break;

    case TryingStatic:
        checkStaticAttempt();
        break;

    case ConnectedStatic:
        checkStaticStillOk();
        break;
    }
}

void
MsgTube::startSTAConnect(bool isSecondary)
{    
    Log.printf("MT: startSTAConnect(isSecondary=%s)\n", isSecondary ? "true": "false");
    if ((NodeConfig.nodeId() == MTMasterAddr+1) && isSecondary)
    {
        // There is no secondary, but we might want to try again for the primary
        startSTANothingPeriod();
        return;
    }

    // Start by making sure we are consistently disconnected
    Log.printf("MT: SSC WiFi.disconnect(true)\n");
    WiFi.disconnect(true);

    if (NodeConfig.nodeId() == MTMasterAddr)
    {
        // Special case for master node, disabled
        staStatus = Disabled;
        nextSTACheck = (uint32_t)-1;
        Log.printf("MT: SSC isMaster, disabling sta connect, return\n");
        return;
    }

    // Configure the next node towards 0
    uint8_t nextNodeId = isSecondary ? NodeConfig.nodeId() - 2 : NodeConfig.nodeId() - 1;
    char szNextNodeName[strlen(NodeConfig.wifiBaseName()) + 3];
    sprintf(szNextNodeName, "%s%d", NodeConfig.wifiBaseName(), nextNodeId);

    Log.printf("MT: SSC WiFi.begin(%s)\n", szNextNodeName);
    WiFi.begin(szNextNodeName, NodeConfig.wifiPassword());

    // Start checking status to see if we get connected
    staAttemptBegan = millis();
    staStatus = isSecondary ? TryingSecondary : TryingPrimary;
    nextSTACheck = staAttemptBegan + STA_Attempt_Check_Delay;
    staCheckAttempts = 0;
    Log.printf("MT: SSC staAttemptBegan=%d now we wait...\n", staAttemptBegan);
}

void
MsgTube::checkSTAAttempt(bool isSecondary)
{
    wl_status_t wifiStatus = WiFi.status();

    staCheckAttempts++;
    if (staCheckAttempts == 1)
    {
        // First time
        Log.printf("MT: checkSTAAttempt(isSecondary=%s) wifiStatus=%d\n", isSecondary ? "true": "false", wifiStatus);
    }
    else
    {        
        Log.printf("%d ", wifiStatus);
    }

    if (staCheckAttempts == 40)
    {
        Log.print("\n");
        staCheckAttempts = 1;
    }
    nextSTACheck = millis() + STA_Attempt_Check_Delay;


    // This is the only thing that means it's all good
    if (WL_CONNECTED == wifiStatus) {
        Log.printf("MT: Connected!!!\n");
        staStatus = isSecondary ? ConnectedSecondary : ConnectedPrimary;
        nextSTACheck = millis() + STA_Connected_Recheck_Delay;

        // Create the local connection
        AsyncClient* client = new AsyncClient();
        IPAddress peer(NodeConfig.apBaseIP());
        peer[2] += isSecondary ? NodeConfig.nodeId() - 2 : NodeConfig.nodeId() - 1;

        // touch it yet
        Log.printf("MT: Setting onConnect and onError handlers for new client\n");
        client->onConnect(std::bind(&MsgTube::sConnect, this, std::placeholders::_1, std::placeholders::_2), NULL);
        client->onError(std::bind(&MsgTube::conError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);

        if (!client->connect(peer, MTServerPort))
        {
            // Aack!
            Log.printf("MT: Unable to create AsyncClient so bailing to Nothing\n");
            startSTANothingPeriod();
            return;
        }

        // MTClientConnection* conn = new MTClientConnection();
        // if (!conn->begin(client))
        // {
        //     Log.printf("MT: Failed to create MTClientConnection for STA");
        //     delete conn;
        // }
        // addConnection(conn);

        Log.printf("MT: New client connection for STA. Yay!!\n");

        return;
    }

    // Anything that is not an error is just "disconnected"
    if (WL_DISCONNECTED == wifiStatus) {
        // How long has it been?
        if (millis() - staAttemptBegan < STA_Attempt_Window) {
            // This is cool, let it keep trying
            return;
        }

        // Aack, timeout. That's the same as an error
    }

    // Some sort of error has occurred

    if (!isSecondary)
    {
        // Move on to secondary
        startSTAConnect(true);
    }
    else
    {
        // It was secondary, so now we must delay
        startSTANothingPeriod();
    }
}

void
MsgTube::startSTANothingPeriod()
{
    Log.printf("MT: startSTANothingPeriod()\n");
    staStatus = Nothing;
    nextSTACheck = millis() + STA_Restart_Delay;    
}

void
MsgTube::checkSTAStillOk(bool isSecondary)
{
    wl_status_t wifiStatus = WiFi.status();

    Log.printf("MT: checkSTAStillOk(isSecondary=%s) wifiStatus=%d\n", isSecondary ? "true": "false", wifiStatus);
    // This is the only thing that means it's all good
    if (WL_CONNECTED == wifiStatus) {
        if (isSecondary)
        {
            // Let's try to connect to Primary. This will probably
            // cause a disruption in network traffic, but eh???
            startSTAConnect(false);
            return;
        }

        // else it's connected to primary, so leave it be
        nextSTACheck = millis() + STA_Connected_Recheck_Delay;
        return;
    }

    // Oh golly, anything else sucks, so we need to re-connect.
    // If we were connected to primary and it failed, does that mean
    // we should immediately try secondary or not? For now, just
    // always restart at nothing.
    staStatus = Nothing;
}



void
MsgTube::startStaticConnect()
{    
    Log.printf("MT: startStaticConnect()\n");

    // Start by making sure we are consistently disconnected
    Log.printf("MT: SSt WiFi.disconnect(true)\n");
    WiFi.disconnect(true);

    // Even if master we must connect to the access point, but
    // if master we use a fixed ip
    if (NodeConfig.nodeId() == MTMasterAddr)
    {
        IPAddress ip( NodeConfig.staticMasterIP() );
        IPAddress gateway(NodeConfig.staticGateway());
        //gateway[3] = 254; // This should move to configuration

        IPAddress subnet(255, 255, 255, 0);

        Log.printf("MT: SSt WiFi.config(%s)\n", ip.toString().c_str());
        WiFi.config(ip, gateway, subnet);
    }

    // Use the base name directly because it is an external AP
    Log.printf("MT: SSt WiFi.begin(%s)\n", NodeConfig.wifiBaseName());
    WiFi.begin(NodeConfig.wifiBaseName(), NodeConfig.wifiPassword());

    // Start checking status to see if we get connected
    staAttemptBegan = millis();
    staStatus = TryingStatic;
    nextSTACheck = staAttemptBegan + STA_Attempt_Check_Delay;
    staCheckAttempts = 0;
    Log.printf("MT: SSt staAttemptBegan=%d now we wait...\n", staAttemptBegan);
}

void
MsgTube::checkStaticAttempt()
{
    wl_status_t wifiStatus = WiFi.status();

    staCheckAttempts++;
    if (staCheckAttempts == 1)
    {
        // First time
        Log.printf("MT: checkSTAAttempt() wifiStatus=%d\n", wifiStatus);
    }
    else
    {        
        Log.printf("%d ", wifiStatus);
    }

    if (staCheckAttempts == 40)
    {
        Log.print("\n");
        staCheckAttempts = 1;
    }
    nextSTACheck = millis() + STA_Attempt_Check_Delay;


    // This is the only thing that means it's all good
    if (WL_CONNECTED == wifiStatus) {
        Log.printf("MT: Static Connected!!!\n");
        staStatus = ConnectedStatic;
        nextSTACheck = millis() + STA_Connected_Recheck_Delay;

        if (useUDP) 
        {
            uint8_t beganUDP = pUDP->begin(MTUDPPort);
            if (beganUDP) 
            {
                Log.printf("MT: Began listening for UDP on %d\n", MTUDPPort);
            }
            else
            {
                Log.printf("MT: Failed to begin UDP\n");
            }
            return;
        }
        else
        {
            if (NodeConfig.nodeId() == MTMasterAddr) {
                Log.printf("MT: Master node, wait for connections from others\n");
                return;
            }

            // We are not the master, so try to connect to it...

            // Create the local connection
            AsyncClient* client = new AsyncClient();
            IPAddress peer(NodeConfig.staticMasterIP()); 

            // touch it yet
            Log.printf("MT: Setting onConnect and onError handlers for new client\n");
            client->onConnect(std::bind(&MsgTube::sConnect, this, std::placeholders::_1, std::placeholders::_2), NULL);
            client->onError(std::bind(&MsgTube::conError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);

            if (!client->connect(peer, MTServerPort))
            {
                // Aack!
                Log.printf("MT: Unable to create AsyncClient so bailing to Nothing\n");
                startSTANothingPeriod();
                return;
            }

            Log.printf("MT: New client connection for Static. Yay!!");
        }

        return;
    }

    // Anything that is not an error is just "disconnected"
    if (WL_DISCONNECTED == wifiStatus) {
        // How long has it been?
        if (millis() - staAttemptBegan < STA_Attempt_Window) {
            // This is cool, let it keep trying
            return;
        }

        // Aack, timeout. That's the same as an error
    }

    // Some sort of error has occurred

    startSTANothingPeriod();
}

void
MsgTube::checkStaticStillOk()
{
    uint32_t before = millis();
    wl_status_t wifiStatus = WiFi.status();
    uint32_t after = millis();

    Log.printf("MT: checkStaticStillOk() wifiStatus=%d time=%d\n", wifiStatus, after-before);
    // This is the only thing that means it's all good
    if (WL_CONNECTED == wifiStatus) {
        // Okay we have wifi.
        nextSTACheck = millis() + STA_Connected_Recheck_Delay;

        // For master & UDP, having wifi is enough to declare happiness
        if (NodeConfig.nodeId() == MTMasterAddr || useUDP) {
            return;
        }

        // For everyone else, we need to have an open MTClientConnection though
        if (!clientConns) {
            Log.printf("MT: CSSO Have wifi but no active MTClientConnection. Restart!");
        } else {
            return;
        }
    }

    // Oh golly, anything else sucks, so we need to re-connect.
    // If we were connected to primary and it failed, does that mean
    // we should immediately try secondary or not? For now, just
    // always restart at nothing.
    staStatus = Nothing;
}


///

void
MsgTube::handleMessage(MTMessage* msg)
{
    if (!msg) return;

    MTListener* cursor = listeners;
    while(cursor)
    {
        if (cursor->handleMTMessage(msg)) return;
        cursor = cursor->nextMTListener;
    }
}

uint32_t MTMessage::nextIdVal = 1000;

/////////

void
MsgTube::receiveUDP()
{
    if (!useUDP || !pUDP) return;

    while(pUDP->parsePacket())
    {
        // There is something to read
        if(udpMsg.readFromUDP(pUDP))
        {
            // Update our routing table to know that the source of this
            // message is reachable at this IP (presuming it's not a broadcast right?)
            if (udpMsg.src < maxNodes)
            {
                IPAddress srcAddr = pUDP->remoteIP();
                // if (srcAddr[3] != 255)
                // {
                    learnedIPs[udpMsg.src] = srcAddr;
                    Log.printf("MT: Learned ip %d.%d.%d.%d for node %d\n", srcAddr[0], srcAddr[1], srcAddr[2], srcAddr[3], udpMsg.src);
                // }
                // else
                // {
                //     Log.printf("MT: Received packet from broadcast addr from node %d\n", udpMsg.src);
                // }
            }

            handleMessage(&udpMsg);
        }
        pUDP->flush();
    }
}

void
MsgTube::sendUDP(MTMessage* msg)
{
    if (!useUDP || !pUDP) return;

    // Figure out an address for the message
    IPAddress address(255, 255, 255, 255); // assume broadcast
    if (msg->dest != MTBroadcastAddr)
    {
        if (msg->dest >= maxNodes)
        {
            Log.printf("MT: ERROR: Peer %d is beyond maxNodes. Can not send via UDP\n");
            return;
        }

        address = learnedIPs[msg->dest];
    }

    pUDP->beginPacket(address, MTUDPPort);

    msg->writeToUDP(pUDP);

    pUDP->endPacket();
}


// We statically declare a message tube for the revolution!
MsgTube msgTube;

