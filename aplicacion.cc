/*
 * aplicacion.cc
 * En este modulo se generan palabras de informaci√≤n del largo dado por el usuario, la cual viaja 
 * a los modulos inferiores para ser enviada al otro Host
 *  Created on: 23-12-2008
 */
#include <string.h>
#include <omnetpp.h>
#include <cstdlib>
#include <iostream>

#define ANADIR_AL_INICIO         0
#define ANADIR_AL_FINAL          1

using namespace std;

class aplicacion : public cSimpleModule
{
  public:
    int cantidad_de_mensajes;
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void generaPalabraInfo(int direccion_de_envio);
    virtual cMessage * AnadirMensajeACMessage(cMessage * msg, char * mensaje, int posicion);
    virtual const char * AplicacionIntToConstChar(int numero, int largo);
};


Define_Module(aplicacion);

void aplicacion::initialize()
{
    int direccion = par("direccion");
    int n_mensajes = par("cantidad_de_mensajes");
    for(int destino = 0; destino < 4; destino++){
        cantidad_de_mensajes = n_mensajes;
        if(destino != direccion){
            for(int i = 0; i < cantidad_de_mensajes; i++)
                generaPalabraInfo(destino);
        }
    }
}

void aplicacion::handleMessage(cMessage *msg)
{
	if (msg->arrivedOn("desde_abajo"))
	{	
		delete msg;//cuando llega un mensaje solo se descarta
	}
	//se genera un mensaje simulando respuesta
}

int LargoInt(int value){
  int l=1;
  while(value>9){ l++; value/=10; }
  return l;
}

cMessage * aplicacion::AnadirMensajeACMessage(cMessage * msg, char * mensaje, int posicion){
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

const char * aplicacion::AplicacionIntToConstChar(int numero, int largo){
    stringstream ss("");
    for(int i = 0; i < largo - LargoInt(numero); i++)
        ss << '0';
    if(numero == 0)
        ss << '0';
    else
        ss << numero;
    string salida = ss.str();
    return salida.c_str();
}

void aplicacion::generaPalabraInfo(int direccion_de_envio)
{
	int direccion = par("direccion");
	int tamT = 4;
	char *mens;

	mens = (char*)malloc(sizeof(char)*tamT);

	//se inicializa la palabra a enviar solo con '0'
	strcpy(mens, "0");
	for(int i=1;i<tamT;i++)
		strcat(mens, "0");
	
        cMessage *palabra = new cMessage(mens);


    // AÒadimos la direccion de envio
    ev << "Host: " << direccion << ": Enviare la palabra " << palabra->getFullName() << " a " << direccion_de_envio << endl;
    palabra = AnadirMensajeACMessage(palabra, (char*)AplicacionIntToConstChar(direccion, 2), ANADIR_AL_INICIO);
    palabra = AnadirMensajeACMessage(palabra, (char*)AplicacionIntToConstChar(direccion_de_envio, 2), ANADIR_AL_INICIO);
	//ev<<"Host "<<direccion<<" - LA PALABRA QUE SE ENVIO DESDE APLICACION ES: "<<palabra->getFullName() << endl;

	send(palabra, "hacia_abajo");//se envia la palabra hacia abajo
}

