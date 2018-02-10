#import "pinger.h"
#import "log.h"

const uint32_t MaxWait = 10000;

Pinger::Pinger() :
    toPing(-1),
    pingInterval(-1),
    lastId(0),
    lastPingAt(0),
    inFlight(false),
    msg(NULL)
{

}

void
Pinger::begin()
{
    if (!msg)
    {
        msg = msgTube.newMessage(4);

        msgTube.addListener(this);
    }
}

void
Pinger::loop()
{    
    uint32_t now = millis();

    if (inFlight)
    {
        if (now - lastPingAt > MaxWait)
        {
            Log.printf("PINGER: Timeout to %d\n", toPing);
            inFlight = false;
            lastPingAt = now;
        }
    }
    else
    {
        if (toPing != -1 && now - lastPingAt > pingInterval)
        {
            // Do a ping!
            msg->incrementId();

            msg->setData((uint8_t*)&now, 4);

            lastId = msg->getId();
            lastPingAt = now;
            inFlight = true;
            Log.printf("PINGER: ping(%d) to %d!\n", msg->getId(), msg->getDest());
            msgTube.sendMessage(msg);
        }
    }
}

void
Pinger::startPings(uint8_t dest, uint32_t interval)
{
    if (!msg)
    {
        begin();
    }

    toPing = dest;
    pingInterval = interval;

    msg->setDest(toPing);
    msg->setType(MT_Type_Ping);
}

bool
Pinger::handleMTMessage(MTMessage* msgIn)
{
    if (!msgIn) return true;

    if (msgIn->isBroadcast()) return false;

    uint32_t now = millis();
    if (msgIn->getType() == MT_Type_Ping)
    {
        // Answer it with a pong
        msgIn->setType(MT_Type_Pong);
        msgIn->swapSrcDest();

        msgIn->setData((uint8_t*)&now, 4);

        Log.printf("PINGER: pong(%d) back to %d\n", msgIn->getId(), msgIn->getDest());
        msgTube.sendMessage(msgIn);
        return true;        
    }

    if (msgIn->getType() == MT_Type_Pong)
    {
        if (!inFlight) return true;

        if (msgIn->getId() != lastId)
        {
            Log.printf("PINGER: Id mismatch, got %d wanted %d\n", msgIn->getId(), lastId);
            return true;
        }

        Log.printf("PINGER: Got pong from %d in %d\n", msgIn->getSrc(), now - lastPingAt);
        lastPingAt = now;
        inFlight = false;
        return true;        
    }


    // Not for me
    return false;
}