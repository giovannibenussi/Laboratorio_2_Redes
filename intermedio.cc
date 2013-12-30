/*
 * intermedio.cc
 *
 *  Created on: 23-12-2008
 *      Author: alan
 */

#include <string.h>
#include <omnetpp.h>

class intermedio : public cSimpleModule
{
  protected:
    virtual void processMsgFromHigherLayer(cMessage *packet);
    virtual void processMsgFromLowerLayer(cMessage *packet);
    virtual void handleMessage(cMessage *msg);
};

Define_Module( intermedio );

void intermedio::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("desde_abajo"))
        processMsgFromLowerLayer(msg);
    else
        processMsgFromHigherLayer(msg);
}

void intermedio::processMsgFromHigherLayer(cMessage *packet)
{
    send(packet,"hacia_abajo");
}

void intermedio::processMsgFromLowerLayer(cMessage *packet)
{
    send(packet,"hacia_arriba");
}
