/*
 * enlace.cc
 * Se reenvian las palabreas que vienen del nivel superior al canal para que viaje al otro host
 *  Created on: 23-12-2008
 *  
 */

#include <string.h>
#include <omnetpp.h>
#include <vector>

#define ANADIR_AL_INICIO         0
#define ANADIR_AL_FINAL          1

// Control de Acceso
#define INICIO_CONTROL_ACCESO    0
#define LARGO_CONTROL_ACCESO     3

// Control de Trama
#define INICIO_CONTROL_TRAMA     3
#define LARGO_CONTROL_TRAMA      3

// Direccion de Destino
#define INICIO_DIRECCION_DESTINO 6
#define LARGO_DIRECCION_DESTINO  2

// Direccion de Origen
#define INICIO_DIRECCION_ORIGEN  8
#define LARGO_DIRECCION_ORIGEN   2

// Datos
#define INICIO_DATOS             10
#define LARGO_DATOS              8

// FCS
#define INICIO_FCS               18
#define LARGO_FCS                3

#define CONTROL_ACCESO_TOKEN "000"
#define CONTROL_ACCESO_TRAMA "001"

#define COLOR_RED     0
#define COLOR_GREEN   1
#define COLOR_BLUE    2
#define COLOR_WHITE   3
#define COLOR_YELLOW  4
#define COLOR_CYAN    5
#define COLOR_MAGENTA 6
#define COLOR_BLACK   7
#define COLOR_TRAMA COLOR_BLACK
#define COLOR_MENSAJE COLOR_RED

using namespace std;

class enlace : public cSimpleModule
{
  protected:
        vector<cMessage*> mensajes;
            virtual void processMsgFromHigherLayer(cMessage *dato);
            virtual void processMsgFromLowerLayer(cMessage *packet);
            virtual void handleMessage(cMessage *msg);
            virtual cMessage * AnadirMensajeACMessage(cMessage * msg, char * mensaje, int posicion);
            virtual cMessage * AnadirMensajeACMessageI(cMessage * msg, char * mensaje, int posicion);
            virtual char * ObtenerMensaje(cMessage * msg, int inicio, int largo);
            virtual char * ObtenerMensajeCharPointer(char * msg, int inicio, int largo);
            virtual bool EsToken(cMessage * msg);
            bool PoseeTrama = false;
            bool EnvieToken = false;
            bool MensajeEnCamino = false;
};

Define_Module( enlace );

char * enlace::ObtenerMensaje(cMessage * msg, int inicio, int largo){
    char * mensaje                = (char*) malloc( sizeof(char) * (largo + 1));
    const char * mensaje_recivido       = msg->getFullName();
    // int    largo_mensaje_recivido = strlen(mensaje_recivido);

    for(int i = 0; i < largo; i++){
        mensaje[i] = mensaje_recivido[ i + inicio ];
    }
    mensaje[ largo ] = '\0';
    return mensaje;
}

char * enlace::ObtenerMensajeCharPointer(char * msg, int inicio, int largo){
    char * mensaje                = (char*) malloc( sizeof(char) * (largo + 1));
    char * mensaje_recivido       = msg;
    // int    largo_mensaje_recivido = strlen(mensaje_recivido);

    for(int i = 0; i < largo; i++){
        mensaje[i] = mensaje_recivido[ i + inicio ];
    }

    mensaje[ largo ] = '\0';
    return mensaje;
}

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

int ObtenerIntDeTrama(cMessage * &dato, int inicio, int largo){
    const char * msg = dato->getFullName();
    int largo_anterior = strlen(msg);
    char * mensaje_nuevo = (char*)malloc(sizeof(char)*(largo_anterior));
    strcpy(mensaje_nuevo, msg);
    int resultado = 0;
    for(int i = inicio; i < inicio + largo; i++){
        int a = mensaje_nuevo[ i ] - '\0' - 48;
        resultado = resultado*10 + a;
    }
    return resultado;
}

void enlace::handleMessage(cMessage *msg)
{
    ev << "MMEnsaje: " << msg->getFullName() << endl;
    if (msg->arrivedOn("desde_fisico")) processMsgFromLowerLayer(msg);//cuando llega un mensaje desde el otro host
    else processMsgFromHigherLayer(msg);//sino es que llega desde arriba
}

bool enlace::EsToken(cMessage * msg){
    if(ObtenerMensaje(msg, INICIO_CONTROL_ACCESO, LARGO_CONTROL_ACCESO) == CONTROL_ACCESO_TOKEN)
        return true;
    return false;
}

//lo que se hace cuando llega una palabra de informaci√≤n desde un modulo superior
void enlace::processMsgFromHigherLayer(cMessage *dato)
{
    ev << "__Mensaje: " << dato->getFullName() << endl;
    int direccion = par("direccion");

    // AÒado los bits de control de trama
    dato = AnadirMensajeACMessage(dato, "000", ANADIR_AL_INICIO);

    // Indico que es una trama
    dato = AnadirMensajeACMessage(dato, CONTROL_ACCESO_TRAMA, ANADIR_AL_INICIO);
    // si soy el host 0 y no se ha enviado nunca el token, comienzo a enviarlo

    if( direccion == 0 && !EnvieToken){
        EnvieToken = true;
        dato->setKind(COLOR_TRAMA);
    }else {
       dato->setKind(COLOR_MENSAJE);
    }

    // int limite_de_tramas = par("limite_de_tramas");

    ev << "__Mensaje: " << dato->getFullName() << endl;

    mensajes.push_back(dato);

    ev << "Mensajes: " << endl;
    for(int i = 0; i < mensajes.size(); i++)
        ev << "\t" << (mensajes.at(i))->getFullName() << endl;
    if(!MensajeEnCamino && mensajes.size() > 0)
        send(mensajes.at(mensajes.size() - 1),"hacia_fisico");
    MensajeEnCamino = true;
}

//lo que se hace cuando llega una palabra de codigo desde otro host
void enlace::processMsgFromLowerLayer(cMessage *packet)
{
    int direccion = par("direccion");
    ev << direccion << " == " << ObtenerIntDeTrama(packet, INICIO_DIRECCION_DESTINO, LARGO_DIRECCION_DESTINO) << endl;
    if(ObtenerIntDeTrama(packet, INICIO_DIRECCION_DESTINO, LARGO_DIRECCION_DESTINO) == direccion){
        ev << "Es para mi !!!! :D" << endl;

        // Modifico el bit del mensaje...

        //mensajes_enviados++;
        ev << "Reenvio y modifico" << endl;
        send(packet, "hacia_fisico");

    }else if(ObtenerIntDeTrama(packet, INICIO_DIRECCION_ORIGEN, LARGO_DIRECCION_ORIGEN) == direccion){
        // si yo lo envie previamente
        MensajeEnCamino = false;
        ev << "se me devolvio !!!" << endl;
        if(mensajes.size() > 0){
            MensajeEnCamino = true;
            mensajes.pop_back();
            if(mensajes.size() > 0)
                send(mensajes.at(mensajes.size() - 1),"hacia_fisico");
        }
    }
    else{
        ev << "No es mio, reenvio..." << endl;
        send(packet, "hacia_fisico");

    }
}
