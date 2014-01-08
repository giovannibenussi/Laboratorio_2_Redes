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
#define COLOR_TOKEN               COLOR_BLACK
#define COLOR_MENSAJE_NO_RECIBIDO COLOR_YELLOW
#define COLOR_MENSAJE_RECIBIDO    COLOR_GREEN

using namespace std;

class enlace : public cSimpleModule
{
    public:
        int limite_de_mensajes = 1;
        int mensajes_enviados  = 0;
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
            void VerEstadisticas();
            bool PoseeTrama      = false;
            bool EnvieToken      = false;
            bool TengoElToken    = false;
            bool MensajeEnCamino = false;
};

Define_Module( enlace );

void enlace::initialize(){
    int direccion = par("direccion");
    TengoElToken = direccion == 0 ? true : false;
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

    // int limite_de_tramas = par("limite_de_tramas");

    ev << "__Mensaje: " << dato->getFullName() << endl;

    mensajes.push_back(dato);

    if(!MensajeEnCamino){
        // si llegue al limite de mensajes, envio el token
        if(mensajes_enviados == limite_de_mensajes){
            ev << "_____________________________" << endl;
            mensajes_enviados = 0;
            EnvieToken = true;
            EnviarToken();
        }else{
            EnviarMensaje();
        }
    }
}

bool enlace::EnviarToken(){
    if(TengoElToken){
        EnvieToken = true;
        TengoElToken = false;
        ev << "Enviare el token" << endl;
        cMessage * token = new cMessage(CONTROL_ACCESO_TOKEN);
        token->setKind(COLOR_TOKEN);
        send(token, "hacia_fisico");
        return true;
    }
    return false;
}

// Intenta enviar un mensaje, si no hay mensajes que enviar, devuelve false
bool enlace::EnviarMensaje(){
    if(mensajes.size() == 0){
        cMessage * paquete = new cMessage("");
        send(paquete, "hacia_arriba");
        return false;
    }
    if(mensajes.size() > 0 && !MensajeEnCamino && TengoElToken){
        int ultimo_elemento = mensajes.size() - 1;
        cMessage * ultimo_mensaje = mensajes.at( ultimo_elemento );
        ultimo_mensaje->setKind(COLOR_MENSAJE_NO_RECIBIDO);
        MensajeEnCamino = true;
        mensajes.pop_back();
        send( ultimo_mensaje ,"hacia_fisico");
        ev << "Enviando Mensaje: " << ultimo_mensaje->getFullName()  << endl;
        this->mensajes_enviados++;
        VerEstadisticas();
        return true;
    }
    return false;
}

void enlace::VerEstadisticas(){
    ev << "Mensajes Enviados : " << mensajes_enviados << endl;
    ev << "Limite de Mensajes: " << limite_de_mensajes << endl;
}

//lo que se hace cuando llega una palabra de codigo desde otro host
void enlace::processMsgFromLowerLayer(cMessage *packet)
{
    MensajeEnCamino = false;
    if(EsToken(packet)){
        ev << "Es el token !!!" << mensajes.size() << endl;
        TengoElToken = true;
        mensajes_enviados = 0;
        if(EnviarMensaje()){
            ev << "Enviare nuevos mensajes" << endl;
        }else{
            ev << "No tengo mensajes, reenviare" << endl;
            TengoElToken = false;
            send(packet, "hacia_fisico");
        }
    }else{
        ev << "NO" << endl;
        int direccion = par("direccion");
        ev << direccion << " == " << ObtenerIntDeTrama(packet, INICIO_DIRECCION_DESTINO, LARGO_DIRECCION_DESTINO) << endl;
        if(EsParaMi(packet, direccion)){
            ev << "Es para mi !!!! :D" << endl;

            // Modifico el bit del mensaje...

            //mensajes_enviados++;
            ev << "Reenvio y modifico" << endl;
            packet->setKind(COLOR_MENSAJE_RECIBIDO);
            send(packet, "hacia_fisico");

        }else if(LoEnvieYo(packet, direccion)){
            // si yo lo envie previamente
            ev << "se me devolvio !!!" << endl;

            VerEstadisticas();
            ev << mensajes_enviados << " == " << limite_de_mensajes << endl;
            // si llegue al limite de mensajes, envio el token
            if(mensajes_enviados == limite_de_mensajes){
                ev << "ACAAAAAA" << endl;
                mensajes_enviados = 0;
                EnviarToken();
            }else{
                ev << "CHAOOOOO" << endl;
                EnviarMensaje();
            }
        }
        else{
            ev << "No es mio, reenvio..." << endl;
            send(packet, "hacia_fisico");

        }
    }
}
