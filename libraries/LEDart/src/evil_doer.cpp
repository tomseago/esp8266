#include "evil_doer.h"

#include "nexus.h"

void
EvilDoer::begin()
{

}

void
EvilDoer::loop()
{
    uint32_t now = millis();

    if (now-lastActionAt > 2000)
    {
        lastActionAt = now;
        doNextAction();
    }
}

enum Action {
    NextAnim,
    NextGeom,
    NextPalette,

    EndMarker
};

Action actionSequence[] = {
    NextAnim,
    NextAnim,
    NextAnim,

    NextGeom,

    NextAnim,
    NextAnim,
    NextAnim,

    NextGeom,

    EndMarker
};
    

void
EvilDoer::doNextAction()
{
    uint32_t now = millis();
    Log.printf("Evil: %d %d ", now, nextActionIx);

    Action action = actionSequence[nextActionIx++];

    switch(action) {
    case NextAnim:
        Log.printf("NextAnim\n");
        nexus.nextAnimation(1234);
        break;

    case NextGeom:
        Log.printf("NextGeom\n");
        nexus.nextGeometry(1234);
        break;

    case NextPalette:
        Log.printf("NextPalette\n");
        nexus.nextPalette(1234);
        break;

    default:
        Log.printf("EndMarker\n");
        nextActionIx = 0;
        lastActionAt = 0;
    }
}