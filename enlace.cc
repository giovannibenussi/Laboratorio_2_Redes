/*
 * enlace.cc
 * Se reenvian las palabreas que vienen del nivel superior al canal para que viaje al otro host
 *  Created on: 23-12-2008
 *  
 */

#include <string.h>
#include <omnetpp.h>

#define ANADIR_AL_INICIO 0
#define ANADIR_AL_FINAL  1

class enlace : public cSimpleModule
{
  protected:
    	virtual void processMsgFromHigherLayer(cMessage *dato);
    	virtual void processMsgFromLowerLayer(cMessage *packet);
    	virtual void handleMessage(cMessage *msg);
    	virtual cMessage * AnadirMensajeACMessage(cMessage * msg, char * mensaje, int posicion);
};

Define_Module( enlace );

cMessage * enlace::AnadirMensajeACMessage(cMessage * msg, char * mensaje, int posicion){
    const char * a_mensaje_anterior = msg->getFullName();
    int largo_total = 1 + strlen(a_mensaje_anterior) + strlen(mensaje);
    char * mensaje_nuevo = (char*) malloc(sizeof(char)*largo_total);
    if(posicion == ANADIR_AL_FINAL){
        strcpy(mensaje_nuevo, a_mensaje_anterior);
        strcat(mensaje_nuevo, mensaje);
    }else{
        strcpy(mensaje_nuevo, mensaje);
        strcat(mensaje_nuevo, a_mensaje_anterior);
    }
    cMessage * nuevo = new cMessage(mensaje_nuevo);
    return nuevo;
}

void enlace::handleMessage(cMessage *msg)
{
    	if (msg->arrivedOn("desde_fisico")) processMsgFromLowerLayer(msg);//cuando llega un mensaje desde el otro host
    	else processMsgFromHigherLayer(msg);//sino es que llega desde arriba
}

//lo que se hace cuando llega una palabra de informaciÃ²n desde un modulo superior
void enlace::processMsgFromHigherLayer(cMessage *dato)
{
    // Ejemplo de como añadir un dato al final del mensaje
    dato = AnadirMensajeACMessage(dato, "0101", ANADIR_AL_FINAL);
    send(dato,"hacia_fisico");
}
//lo que se hace cuando llega una palabra de codigo desde otro host
void enlace::processMsgFromLowerLayer(cMessage *packet)
{
    	send(packet,"hacia_arriba");
}
