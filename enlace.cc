/*
 * enlace.cc
 * Se reenvian las palabreas que vienen del nivel superior al canal para que viaje al otro host
 *  Created on: 23-12-2008
 *  
 */

#include <string.h>
#include <omnetpp.h>

#define ANADIR_AL_INICIO         0
#define ANADIR_AL_FINAL          1
#define INICIO_CONTROL_ACCESO    0
#define LARGO_CONTROL_ACCESO     1
#define INICIO_CONTROL_TRAMA     1
#define LARGO_CONTROL_TRAMA      1
#define INICIO_DIRECCION_DESTINO 2
#define LARGO_DIRECCION_DESTINO  1
#define INICIO_DIRECCION_ORIGEN  3
#define LARGO_DIRECCION_ORIGEN   1
#define INICIO_DATOS             4
#define LARGO_DATOS              4
#define INICIO_FCS               8
#define LARGO_FCS                1

#define CONTROL_ACCESO_TOKEN "0"
#define CONTROL_ACCESO_TRAMA "1"

#define COLOR_RED     0
#define COLOR_GREEN   1
#define COLOR_BLUE    2
#define COLOR_WHITE   3
#define COLOR_YELLOW  4
#define COLOR_CYAN    5
#define COLOR_MAGENTA 6
#define COLOR_BLACK   7
#define COLOR_TRAMA COLOR_BLACK

class enlace : public cSimpleModule
{
  protected:
    	virtual void processMsgFromHigherLayer(cMessage *dato);
    	virtual void processMsgFromLowerLayer(cMessage *packet);
    	virtual void handleMessage(cMessage *msg);
    	virtual cMessage * AnadirMensajeACMessage(cMessage * msg, char * mensaje, int posicion);
    	virtual cMessage * AnadirMensajeACMessageI(cMessage * msg, char * mensaje, int posicion);
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

cMessage * enlace::AnadirMensajeACMessageI(cMessage * msg, char * mensaje, int inicio){
    const char * a_mensaje_anterior = msg->getFullName();
    int largo_total = 1 + strlen(a_mensaje_anterior) + strlen(mensaje);
    int largo_mensaje = strlen(mensaje);
    char * mensaje_nuevo = (char*) malloc(sizeof(char)*largo_total);
    strcpy(mensaje_nuevo, "");
    for(int i = 0; i < inicio; i++){

        char * caracter = (char*)malloc(sizeof(char));
        *caracter = a_mensaje_anterior[i];
        const char * puntero = caracter;
        strcat(mensaje_nuevo, puntero);
        ev << "Copie: " << a_mensaje_anterior[i] << endl;
    }
    strcat(mensaje_nuevo, mensaje);
    ev << "Copie: " << mensaje << endl;
    for(int i = inicio + largo_mensaje; i < largo_total; i++){
        char * caracter = (char*)malloc(sizeof(char));
        *caracter = a_mensaje_anterior[i];
        const char * puntero = caracter;
        strcat(mensaje_nuevo, puntero);
        ev << "_Copie: " << a_mensaje_anterior[i] << endl;
    }
    mensaje_nuevo[largo_total] = '\0';
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
    dato->setKind(COLOR_TRAMA);
    // dato = AnadirMensajeACMessageI(dato, "9", 2);
    send(dato,"hacia_fisico");
}
//lo que se hace cuando llega una palabra de codigo desde otro host
void enlace::processMsgFromLowerLayer(cMessage *packet)
{
    	send(packet,"hacia_arriba");
}
