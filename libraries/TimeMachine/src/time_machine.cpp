#include "time_machine.h"

TimeMachine::TimeMachine() :
    _encYear(27,14,13)
{

}

void
TimeMachine::begin()
{
    _lcd.begin();

    _encYear.begin();
}

void
TimeMachine::loop()
{
    _encYear.loop();

    _lcd.loop();
}