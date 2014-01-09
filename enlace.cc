/*
 * enlace.cc
 * Se reenvian las palabreas que vienen del nivel superior al canal para que viaje al otro host
 *  Created on: 23-12-2008
 *  
 */

#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <time.h>
#include <stdlib.h>

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

#define TRAMA_DATOS "000"
#define TRAMA_ACK   "001"

#define COLOR_RED     0
#define COLOR_GREEN   1
#define COLOR_BLUE    2
#define COLOR_WHITE   3
#define COLOR_YELLOW  4
#define COLOR_CYAN    5
#define COLOR_MAGENTA 6
#define COLOR_BLACK   7
#define COLOR_TOKEN               COLOR_BLACK
#define COLOR_MENSAJE_NO_RECIBIDO COLOR_YELLOW
#define COLOR_MENSAJE_RECIBIDO    COLOR_GREEN
#define COLOR_ACK                 COLOR_BLUE

#define CON_ERROR 1
#define SIN_ERROR 0

using namespace std;

class enlace : public cSimpleModule
{
    public:
        int limite_de_mensajes = 2;
        int mensajes_enviados  = 0;
        int ultimo_recibido    = 0;
        int mensajes_confirmados = 0;
        int probabilidad_de_error = 50;
  protected:
        vector<cMessage*> mensajes;
            virtual void initialize();
            virtual void processMsgFromHigherLayer(cMessage *dato);
            virtual void processMsgFromLowerLayer(cMessage *packet);
            virtual void handleMessage(cMessage *msg);
            virtual cMessage * AnadirMensajeACMessage(cMessage * msg, char * mensaje, int posicion);
            virtual cMessage * AnadirMensajeACMessageI(cMessage * msg, char * mensaje, int posicion);
            virtual char * ObtenerMensaje(cMessage * msg, int inicio, int largo);
            virtual char * ObtenerMensajeCharPointer(char * msg, int inicio, int largo);
            bool EsParaMi(cMessage * packet, int direccion);
            bool LoEnvieYo(cMessage * packet, int direccion);
            bool EsToken(cMessage * msg);
            bool HayMensajes();
            bool EnviarMensaje();
            bool EnviarToken();
            bool EsAck(cMessage * packet);
            bool EsMensaje(cMessage * packet);
            cMessage * FCS(cMessage * packet, int error);
            bool ComprobarFcs(cMessage * packet);
            void VerEstadisticas();
            int LargoInt(int value);
            const char * IntToConstChar(int numero, int largo);
            cMessage * CrearACK(int i, int origen, int destino);
            bool PoseeTrama      = false;
            bool EnvieToken      = false;
            bool TengoElToken    = false;
            bool MensajeEnCamino = false;
};

Define_Module( enlace );

void enlace::initialize(){
    srand(time(NULL));
    int direccion = par("direccion");
    TengoElToken = direccion == 1 ? true : false;
}

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
    }
    strcat(mensaje_nuevo, mensaje);
    for(int i = inicio + largo_mensaje; i < largo_total; i++){
        char * caracter = (char*)malloc(sizeof(char));
        *caracter = a_mensaje_anterior[i];
        const char * puntero = caracter;
        strcat(mensaje_nuevo, puntero);
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
    if (msg->arrivedOn("desde_fisico")) processMsgFromLowerLayer(msg);//cuando llega un mensaje desde el otro host
    else processMsgFromHigherLayer(msg);//sino es que llega desde arriba
}

bool enlace::HayMensajes(){
    return mensajes.size() > 0;
}

bool enlace::EsParaMi(cMessage * packet, int direccion){
    return ObtenerIntDeTrama(packet, INICIO_DIRECCION_DESTINO, LARGO_DIRECCION_DESTINO) == direccion;
}

bool enlace::LoEnvieYo(cMessage * packet, int direccion){
    return ObtenerIntDeTrama(packet, INICIO_DIRECCION_ORIGEN, LARGO_DIRECCION_ORIGEN) == direccion;
}

bool enlace::EsToken(cMessage * msg){
    if(strcmp(ObtenerMensaje(msg, INICIO_CONTROL_ACCESO, LARGO_CONTROL_ACCESO), CONTROL_ACCESO_TOKEN) == 0)
        return true;
    return false;
}

//lo que se hace cuando llega una palabra de informaciÃ²n desde un modulo superior
void enlace::processMsgFromHigherLayer(cMessage *dato)
{
    int direccion = par("direccion");

    // Añado los bits de control de trama
    dato = AnadirMensajeACMessage(dato, "000", ANADIR_AL_INICIO);

    // Indico que es una trama
    dato = AnadirMensajeACMessage(dato, CONTROL_ACCESO_TRAMA, ANADIR_AL_INICIO);

    cMessage * copia = dato;
    copia = FCS(copia, SIN_ERROR);
    dato = FCS(dato, CON_ERROR);

    mensajes.push_back(dato);

    if(!MensajeEnCamino){
        // si llegue al limite de mensajes, envio el token
        if(mensajes_enviados == limite_de_mensajes){
            //mensajes_enviados = 0;
            EnvieToken = true;
            EnviarToken();
        }else{
            EnviarMensaje();
        }
    }
}

bool enlace::EnviarToken(){
    if(TengoElToken && (mensajes_confirmados == mensajes_enviados || mensajes.size() == 0 )){
        EnvieToken = true;
        TengoElToken = false;
        cMessage * token = new cMessage(CONTROL_ACCESO_TOKEN);
        token->setKind(COLOR_TOKEN);
        mensajes_enviados    = 0;
        mensajes_confirmados = 0;
        send(token, "hacia_fisico");
        return true;
    }
    return false;
}

// Intenta enviar un mensaje, si no hay mensajes que enviar, devuelve false
bool enlace::EnviarMensaje(){
    /*if(mensajes.size() == 0){
        cMessage * paquete = new cMessage("");
        send(paquete, "hacia_arriba");
        ev << "Enviados: " << mensajes_enviados << endl;
            ev << "Confirmados:  "<< mensajes_confirmados << endl;
        return false;
    }*/
    int a = 10;
    while(mensajes.size() > 0 && !MensajeEnCamino && TengoElToken && mensajes_enviados < limite_de_mensajes){
        int ultimo_elemento = mensajes.size() - 1;
        cMessage * ultimo_mensaje = mensajes.at( ultimo_elemento );
        ultimo_mensaje->setKind(COLOR_MENSAJE_NO_RECIBIDO);
        // si no es correcto
        if(!ComprobarFcs(ultimo_mensaje)){
            ultimo_mensaje->setKind(COLOR_MAGENTA);
        }
        // MODIFICADO
        MensajeEnCamino = false;
        mensajes.pop_back();
        send( ultimo_mensaje ,"hacia_fisico");
        mensajes_enviados++;
        //return true;
    }
    return false;
}

int enlace::LargoInt(int value){
    int l=1;
    while(value>9){ l++; value/=10; }
    return l;
}

const char * enlace::IntToConstChar(int numero, int largo){
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

cMessage * enlace::FCS(cMessage * packet, int error){
    char * mensaje = (char*)packet->getFullName();
    int largo = strlen(mensaje);
    int suma = 0;
    for(int i = 0; i < largo; i++){
        suma += mensaje[i] % 256;
        suma %= 256;
    }
    int random = (rand() %101);
    if(error == CON_ERROR && probabilidad_de_error < random){
        suma += 123;
        suma %= 256;
    }

    //return AnadirMensajeACMessage(packet, (char*)IntToConstChar(suma, LARGO_FCS), ANADIR_AL_FINAL);
    //packet = AnadirMensajeACMessage(packet, (char*)IntToConstChar(suma, LARGO_FCS), ANADIR_AL_INICIO);
    char * mensaje_nuevo = (char*) malloc( sizeof(char) * (largo + LARGO_FCS) );
    strcpy(mensaje_nuevo, mensaje);
    strcat(mensaje_nuevo, IntToConstChar(suma, LARGO_FCS));
    cMessage * paquete_nuevo = new cMessage(mensaje_nuevo);
    return paquete_nuevo;
}

bool enlace::ComprobarFcs(cMessage * packet){
    char * mensaje = (char*)packet->getFullName();
    int largo = strlen(mensaje);
    int largo_sin_fcs = largo - LARGO_FCS;
    char * mensaje_sin_fcs = (char*) (malloc( sizeof(char) * ( largo_sin_fcs ) ));
    for(int i = 0; i < largo_sin_fcs; i++){
        mensaje_sin_fcs[ i ] = mensaje[ i ];
    }
    mensaje_sin_fcs[ largo_sin_fcs ] = '\0';
    cMessage * mensaje_sin = new cMessage(mensaje_sin_fcs);
    mensaje_sin = FCS(mensaje_sin, SIN_ERROR);
    return strcmp(mensaje_sin->getFullName(), packet->getFullName()) == 0;
}

cMessage * enlace::CrearACK(int i, int origen, int destino){
    // Contenido del mensaje
    cMessage * ack = new cMessage((char*)IntToConstChar(i, LARGO_DATOS));
    // Mi Direccion
    ack = AnadirMensajeACMessage(ack, (char*)IntToConstChar(origen, LARGO_DIRECCION_ORIGEN), ANADIR_AL_INICIO);
    // Direccion de Envio
    ack = AnadirMensajeACMessage(ack, (char*)IntToConstChar(destino, LARGO_DIRECCION_DESTINO),ANADIR_AL_INICIO);
    // Tipo de mensaje (ack, datos, etc).
    ack = AnadirMensajeACMessage(ack, (char*)TRAMA_ACK, ANADIR_AL_INICIO);
    // Tipo de mensaje (token o datos)
    ack = AnadirMensajeACMessage(ack, (char*)CONTROL_ACCESO_TRAMA, LARGO_CONTROL_ACCESO);
    // Añado fcs a la trama
    ack = FCS(ack, SIN_ERROR);
    ack->setKind(COLOR_ACK);
    return ack;
}

bool enlace::EsAck(cMessage * packet){
    return strcmp(ObtenerMensaje(packet, INICIO_CONTROL_TRAMA, LARGO_CONTROL_TRAMA), TRAMA_ACK) == 0;
}

bool enlace::EsMensaje(cMessage * packet){
    return strcmp(ObtenerMensaje(packet, INICIO_CONTROL_TRAMA, LARGO_CONTROL_TRAMA), TRAMA_DATOS) == 0;
}

//lo que se hace cuando llega una palabra de codigo desde otro host
void enlace::processMsgFromLowerLayer(cMessage *packet)
{
    MensajeEnCamino = false;
    if(EsToken(packet)){
        TengoElToken = true;
        mensajes_enviados = 0;
        if(mensajes.size() > 0){
            EnviarMensaje();
            ev << "Enviare nuevos mensajes" << endl;
        }else{
            ev << "No tengo mensajes, reenviare" << endl;
            EnviarToken();
            //send(packet, "hacia_fisico");
        }
    }else{
        int direccion = par("direccion");
        if(EsParaMi(packet, direccion)){
            // Modifico el bit del mensaje...

            //packet->setKind(COLOR_MENSAJE_RECIBIDO);
            if(EsAck(packet)){
                mensajes_confirmados++;
                ev << "Me llego un ACK !!!" << endl;
                //send(packet, "hacia_fisico");
                if(mensajes_enviados >= limite_de_mensajes || mensajes.size() == 0){
                    EnviarToken();
                }else{
                    EnviarMensaje();
                }
            }else if(EsMensaje(packet)){
                ev << "Me llego un mensaje !!!, enviare un ACK" << endl;
                ev << packet->getFullName() << endl;
                if(ComprobarFcs(packet)){
                    ev << "Fcs CORRECTO" << endl;
                }else {
                    ev << "Fcs MALO" << endl;
                }
                cMessage * paquete = CrearACK(ultimo_recibido, direccion, ObtenerIntDeTrama(packet, INICIO_DIRECCION_ORIGEN, LARGO_DIRECCION_ORIGEN));
                ultimo_recibido++;
                send(paquete, "hacia_fisico");
            }else{
                ev << "Ni mensaje ni ack" << endl;
            }

        }else if(LoEnvieYo(packet, direccion)){
            // si yo lo envie previamente

            // si llegue al limite de mensajes, envio el token
            if(mensajes_enviados >= limite_de_mensajes){
                //mensajes_enviados = 0;
                EnviarToken();
            }else{
                EnviarMensaje();
            }
        }
        else{
            send(packet, "hacia_fisico");

        }
    }
}
