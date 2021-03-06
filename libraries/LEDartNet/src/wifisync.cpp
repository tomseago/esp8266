#include "WiFiSync.h"
#include "nexus.h"

const uint32_t QueryInterval = 5000;
const int32_t JumpInterval = 20;
const size_t StateSize = 100;

WiFiSync::StatusAnim::StatusAnim(WiFiSync& parent) : 
    LEDArtAnimation("WiFiSyncStatus"),
    parent(parent)
{
    type = LEDAnimationType_STATUS;
    maxDuration = 0;
    loopDuration = 3000;
    ignoreSpeedFactor = true;
}


LAColor red(255,0,0);
LAColor green(0, 255, 0);
LAColor blue(0, 0, 255);

LAColor colors[] = { red, green, blue };

void 
WiFiSync::StatusAnim::animate(LEDArtPiece& piece, LEDAnimationParam p)
{
    return;
    uint8_t offset = (uint8_t)(p.progress * 3);

    for(uint8_t i=0; i<3; i++) {
        uint8_t pix = i+offset;
        if (pix>2) pix -= 3;
        piece.strip.SetPixelColor(pix, colors[i]);
    }
    // piece.strip.SetPixelColor(0, red);
    // piece.strip.SetPixelColor(1, green);
    // piece.strip.SetPixelColor(2, blue);
}

///////


WiFiSync::WiFiSync(Nexus& nexus) :
    statusAnim(*this),
    nexus(nexus)
{

}

void
WiFiSync::begin()
{
    if (msgTube.isMaster()) 
    {
        // Space for 30 bytes data plus names
        stateMsg = msgTube.newMessage(StateSize);
        if (stateMsg)
        {
            stateMsg->setType(MT_Type_PreparedState);
            stateMsg->setDest(MTBroadcastAddr);
        }

        nexus.shouldPrepareRandomStatesFor = 2000;
    } 
    else
    {
        // Let our time sync up
        queryMsg = msgTube.newMessage(sizeof(uint32_t));
        if (queryMsg)
        {
            queryMsg->setType(MT_Type_TimeQuery);
            queryMsg->setDest(MTMasterAddr);
        }

        // Force forever looping so that we only change animations as a result
        // of a prepared change from master
        nexus.forcedForeverLoop = true;
    }

    // Be a listener for messages
    msgTube.addListener(this);
    nexus.addListener(this);

    // This kick in the butt will cause the LEDArtPiece to generate
    // an initial prepared state in case other things are booting
    // at basically the same time and need the data
    if (msgTube.isMaster())
    {
        nexus.nextAnimation((uint32_t)this);
    }
}

void
WiFiSync::loop()
{
    if (msgTube.isMaster()) 
    {
        // If there were updates via the nexus, we need to send out
        // the current state to people
        if (valuesUpdated) 
        {
            Log.printf("WIFISYNC: Sending current state\n");
            sendCurrentState();
            valuesUpdated = false;            
        }
    } 
    else 
    {
        // Query for time updates periodically
        uint32_t now = millis();

        if (now-lastQueryAt > QueryInterval)
        {
            queryTime();
            lastQueryAt = now;
        }
    }

}

bool
WiFiSync::handleMTMessage(MTMessage* msgIn)
{
    if (!msgIn) return true;

    /////////
    // Handle Broadcasts first

    if (msgIn->isBroadcast()) {

        // Only care about PreparedState broadcasts
        if (msgIn->getType() == MT_Type_PreparedState)
        {
            bool success = nexus.deserializeState(false, msgIn->getLength(), msgIn->getData());
            if (!success)
            {
                Log.printf("ERROR: Failed to deserialize prepared state\n");
            }
            else
            {
                Log.printf("WIFISYNC: Got prepared state!\n");
            }
            return true;
        }

        if (msgIn->getType() == MT_Type_CurrentState)
        {
            bool success = nexus.deserializeState(true, msgIn->getLength(), msgIn->getData());
            if (!success)
            {
                Log.printf("ERROR: Failed to deserialize current state\n");
            }
            else
            {
                Log.printf("WIFISYNC: Got a current state\n");
            }
            return true;
        }

        // Not for me
        return false;
    }

    
    /////////
    // It's a unicast for me! What might it be?
    Log.printf("WIFISYNC: Got message type %d\n", msgIn->getType());
    uint32_t now = millis();    

    // Time Query
    if (msgIn->getType() == MT_Type_TimeQuery)
    {
        // Answer it with a pong
        msgIn->setType(MT_Type_TimeResponse);
        msgIn->swapSrcDest();

        msgIn->setData((uint8_t*)&now, sizeof(now));

        Log.printf("WIFISYNC: response(%d) back to %d\n", msgIn->getId(), msgIn->getDest());
        msgTube.sendMessage(msgIn);
        return true;        
    }

    // Time Response
    if (msgIn->getType() == MT_Type_TimeResponse)
    {
        if (msgIn->getId() != lastId)
        {
            Log.printf("WIFISYNC: Id mismatch, got %d wanted %d\n", msgIn->getId(), lastId);
            return true;
        }

        if (msgIn->getLength() != sizeof(unsigned long)) 
        {
            Log.printf("WIFISYNC: Expected %d bytes of data but got %d\n", sizeof(unsigned long), msgIn->getLength());
            return true;
        }        

        uint32_t remoteTime = *((unsigned long *)msgIn->getData());
        int32_t elapsed = now - lastQueryAt;
        uint32_t localAtRemote = now - (elapsed / 2);
        int32_t rawTimeDiff = localAtRemote - remoteTime;
        int32_t adjustedTimeDiff = (localAtRemote + nexus.localTimeOffset) - remoteTime;


        Log.printf("WIFISYNC: Got time response from %d in %d, timeDiff raw=%d adj=%d, lto=%d\n", msgIn->getSrc(), elapsed, rawTimeDiff, adjustedTimeDiff, nexus.localTimeOffset);

        if (labs(adjustedTimeDiff) > JumpInterval)
        {
            nexus.localTimeOffset = -rawTimeDiff;
            Log.printf("WIFISYNC: Adjusted localTimeOffset to -rawDiff\n");
        }

        // Do we know our next state? If not, query for it
        if (!nexus.nextPreparedState(NULL, NULL, NULL))
        {
            Log.printf("WIFISYNC: Time response but no state, querying for it\n");
            queryState();
        }

        return true;        
    }

    // State Query
    if (msgIn->getType() == MT_Type_StateQuery)
    {
        if (!msgTube.isMaster()) 
        {
            Log.printf("WIFISYNC: Refusing to answer state query when not master\n");
            return true;
        }

        sendState();
        return true;        
    }

    return false;
}

void
WiFiSync::nexusValueUpdate(NexusValueType which, uint32_t source)
{
    if (msgTube.isMaster())
    {
        if (which == NexusListener::NexusValueType::PreparedState)
        {
            sendState();
        }
        else
        {
            // Will send current state
            Log.printf("WIFISYNC: Told about updated values %d\n",which);
            valuesUpdated = true;
        }
    }
}

void
WiFiSync::nexusUserAnimationRequest(char* szName, bool randomize, uint32_t source) 
{
    // Don't care...
}

////////

void
WiFiSync::queryState()
{
    if (msgTube.isMaster()) {
        // Do not query if we are the master
        return;
    }

    queryMsg->incrementId();
    queryMsg->setType(MT_Type_StateQuery);
    queryMsg->setData(NULL, 0);

    lastId = queryMsg->getId();
    Log.printf("WIFISYNC: stateQuery(%d) to %d!\n", queryMsg->getId(), queryMsg->getDest());

    msgTube.sendMessage(queryMsg);
}

void
WiFiSync::sendState()
{
    // Send this to our people!
    if (!stateMsg)
    {
        Log.printf("WIFISYNC: No stateMsg. Can not send broadcast\n");
        return;
    }
    stateMsg->setType(MT_Type_PreparedState);

    uint8_t data[StateSize];
    uint16_t len = 0;

    len = nexus.serializeState(false, false, data);
    if (!len)
    {
        Log.printf("WIFISYNC: No prepared state to send\n");
        return;
    }

    stateMsg->incrementId();
    stateMsg->setData(data, len);
    Log.printf("WIFISYNC: Broadcasting my state\n");

    msgTube.sendMessage(stateMsg);    
}

void
WiFiSync::sendCurrentState()
{
    // Send this to our people!
    if (!stateMsg)
    {
        Log.printf("WIFISYNC: No stateMsg. Can not send current state broadcast\n");
        return;
    }
    stateMsg->setType(MT_Type_CurrentState);

    uint8_t data[StateSize];
    uint16_t len = 0;

    len = nexus.serializeState(true, false, data);
    if (!len)
    {
        Log.printf("WIFISYNC: No prepared state to send\n");
        return;
    }

    stateMsg->incrementId();
    stateMsg->setData(data, len);
    Log.printf("WIFISYNC: Broadcasting CURRENT state\n");

    msgTube.sendMessage(stateMsg);    
}

void
WiFiSync::queryTime()
{
    if (msgTube.isMaster()) {
        // Do not query if we are the master
        return;
    }

    unsigned long now = millis();

    queryMsg->incrementId();
    queryMsg->setType(MT_Type_TimeQuery);
    queryMsg->setData((uint8_t*)&now, sizeof(now));

    lastId = queryMsg->getId();
    Log.printf("WIFISYNC: timeQuery(%d) to %d!\n", queryMsg->getId(), queryMsg->getDest());

    msgTube.sendMessage(queryMsg);
}
