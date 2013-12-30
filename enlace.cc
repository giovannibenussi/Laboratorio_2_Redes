/*
 * enlace.cc
 * Se reenvian las palabreas que vienen del nivel superior al canal para que viaje al otro host
 *  Created on: 23-12-2008
 *  
 */

#include <string.h>
#include <omnetpp.h>

class enlace : public cSimpleModule
{
  protected:
    	virtual void processMsgFromHigherLayer(cMessage *dato);
    	virtual void processMsgFromLowerLayer(cMessage *packet);
    	virtual void handleMessage(cMessage *msg);
};

Define_Module( enlace );

void enlace::handleMessage(cMessage *msg)
{
    	if (msg->arrivedOn("desde_fisico")) processMsgFromLowerLayer(msg);//cuando llega un mensaje desde el otro host
    	else processMsgFromHigherLayer(msg);//sino es que llega desde arriba
}

//lo que se hace cuando llega una palabra de informaciòn desde un modulo superior
void enlace::processMsgFromHigherLayer(cMessage *dato)
{
    	send(dato,"hacia_fisico");
}
//lo que se hace cuando llega una palabra de codigo desde otro host
void enlace::processMsgFromLowerLayer(cMessage *packet)
{
    	send(packet,"hacia_arriba");
}
