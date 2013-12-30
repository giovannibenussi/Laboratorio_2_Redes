/*
 * aplicacion.cc
 * En este modulo se generan palabras de informaciòn del largo dado por el usuario, la cual viaja 
 * a los modulos inferiores para ser enviada al otro Host
 *  Created on: 23-12-2008
 */
#include <string.h>
#include <omnetpp.h>
#include <cstdlib>
#include <iostream>
using namespace std;

class aplicacion : public cSimpleModule
{
  public:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void generaPalabraInfo();
};

//se usa para que sòlo uno de los host comience la comunicacion
int turno=0;

Define_Module(aplicacion);

void aplicacion::initialize()
{
	if(turno==0)
	{
		generaPalabraInfo();
		turno=1;
	}
}

void aplicacion::handleMessage(cMessage *msg)
{
	if (msg->arrivedOn("desde_abajo"))
	{	
		delete msg;//cuando llega un mensaje solo se descarta
	}
	//se genera un mensaje simulando respuesta
	generaPalabraInfo();
}

void aplicacion::generaPalabraInfo()
{
	int direccion = par("direccion");
	int tamT = par("tamTrama");
	char *mens;

	mens = (char*)malloc(sizeof(char)*tamT);

	//se inicializa la palabra a enviar solo con '0'
	strcpy(mens, "0");
	for(int i=1;i<tamT;i++)
		strcat(mens, "0");
	
        cMessage *palabra = new cMessage(mens);
        send(palabra, "hacia_abajo");//se envia la palabra hacia abajo

	ev<<"Host "<<direccion<<" - LA PALABRA QUE SE ENVIO DESDE APLICACION ES: "<<mens;
}

