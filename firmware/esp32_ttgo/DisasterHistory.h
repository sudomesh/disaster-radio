#ifndef DISASTERHISTORY_H
#define DISASTERHISTORY_H

#include "DisasterClient.h"

class DisasterHistory
{
public:
    virtual void record(String message) = 0;
    virtual void replay(DisasterClient *client) = 0;
};

#endif