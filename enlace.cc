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
#define LARGO_CONTROL_ACCESO     4

// Control de Trama
#define INICIO_CONTROL_TRAMA     4
#define LARGO_CONTROL_TRAMA      4

// Direccion de Destino
#define INICIO_DIRECCION_DESTINO 8
#define LARGO_DIRECCION_DESTINO  1

// Direccion de Origen
#define INICIO_DIRECCION_ORIGEN  9
#define LARGO_DIRECCION_ORIGEN   1

// Datos
#define INICIO_DATOS             10
#define LARGO_DATOS              6

// FCS
#define INICIO_FCS               16
#define LARGO_FCS                1

#define CONTROL_ACCESO_TOKEN "TKN "
#define CONTROL_ACCESO_TRAMA "DAT "

#define TRAMA_DATOS "DATO"
#define TRAMA_ACK   "ACK "
#define TRAMA_NACK  "NACK"

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
#define COLOR_NACK                COLOR_RED

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
        int mensajes_enviados_totales = 0;
        int base = 0;
        int host;
  protected:
        vector<cMessage*> mensajes;
        vector<cMessage*> mensajes_copia;
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
            void EnviarMensaje();
            bool EnviarToken();
            bool EsAck(cMessage * packet);
            bool EsNack(cMessage * packet);
            bool EsMensaje(cMessage * packet);
            cMessage * FCS(cMessage * packet, int error);
            bool ComprobarFcs(cMessage * packet);
            void VerEstadisticas();
            int LargoInt(int value);
            const char * IntToConstChar(int numero, int largo);
            cMessage * CrearACK(int i, int origen, int destino);
            cMessage * CrearNACK(int i, int origen, int destino);
            void VerMensajes();
            bool PoseeTrama      = false;
            bool EnvieToken      = false;
            bool TengoElToken    = false;
            bool MensajeEnCamino = false;
};

Define_Module( enlace );

void enlace::initialize(){
    // Obtenemos la direccion del host
    host = par("direccion");
    // Esto es para que la funcion rand() retorne un valor diferente en cada instancia de la simulacion
    srand(time(NULL));
    // Ob
    int direccion = par("direccion");
    // si es el host 0, tendrá el token al iniciar la simulación N°i
    TengoElToken = direccion == 0 ? true : false;
}

// Función utilizada para obtener una porción de una trama desde i hasta i + largo
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

// Mismo efecto que la función ObtenerMensaje, a diferencia que esta retorna un
// dato de tipo char *
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

// Añade el texto "mensaje" a la trama "msg".
// Posicion indica si se añade al comienzo o al final de la trama
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

// Función que funciona como la función ObtenerMensaje, a diferencia que este retorna un
// dato de tipo int
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

// Retorna verdadero si hay mensajes en el vector "mensajes"
bool enlace::HayMensajes(){
    return mensajes.size() > 0;
}

// Retorna verdadero si "packet" tiene como direccion de destino la direccion indicada en "direccion"
bool enlace::EsParaMi(cMessage * packet, int direccion){
    return ObtenerIntDeTrama(packet, INICIO_DIRECCION_DESTINO, LARGO_DIRECCION_DESTINO) == direccion;
}

// Retorna verdadero si "packet" tiene como direccion de origen mi direccion
bool enlace::LoEnvieYo(cMessage * packet, int direccion){
    return ObtenerIntDeTrama(packet, INICIO_DIRECCION_ORIGEN, LARGO_DIRECCION_ORIGEN) == direccion;
}

// Retorna verdadero si "msg" es el token
bool enlace::EsToken(cMessage * msg){
    if(strcmp(ObtenerMensaje(msg, INICIO_CONTROL_ACCESO, LARGO_CONTROL_ACCESO), CONTROL_ACCESO_TOKEN) == 0)
        return true;
    return false;
}

// lo que se hace cuando llega una palabra de información desde un módulo superior
void enlace::processMsgFromHigherLayer(cMessage *dato)
{
    int direccion = par("direccion");

    // Añado los bits de control de trama
    dato = AnadirMensajeACMessage(dato, TRAMA_DATOS, ANADIR_AL_INICIO);

    // Indico que es una trama
    dato = AnadirMensajeACMessage(dato, CONTROL_ACCESO_TRAMA, ANADIR_AL_INICIO);

    cMessage * copia = dato;
    copia = FCS(copia, SIN_ERROR);
    dato = FCS(dato, CON_ERROR);

    mensajes.push_back(dato);
    mensajes_copia.push_back(dato);

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

// Función que se encarga de enviar el token a la estación siguiente
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

// Intenta enviar un mensaje
void enlace::EnviarMensaje(){
    int a = 10;
    while(mensajes.size() > 0 && !MensajeEnCamino && TengoElToken && mensajes_enviados < limite_de_mensajes){
        base++;
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
        mensajes_enviados_totales++;
    }
}

// Función que retorna el largo del número "value"
int enlace::LargoInt(int value){
    int l=1;
    while(value>9){ l++; value/=10; }
    return l;
}

// Función que transforma "numero" de tipo int a const char * (de largo "largo")
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

// Añade a "packet" la secuencia de comprobación de trama.
// "error" se utiliza para indicar si no habra error, o, si habra una probabilidad
// de "probabilidad_de_error" de que la trama enviada contenga errores.
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
    suma = suma % 10;

    //return AnadirMensajeACMessage(packet, (char*)IntToConstChar(suma, LARGO_FCS), ANADIR_AL_FINAL);
    //packet = AnadirMensajeACMessage(packet, (char*)IntToConstChar(suma, LARGO_FCS), ANADIR_AL_INICIO);
    char * mensaje_nuevo = (char*) malloc( sizeof(char) * (largo + LARGO_FCS) );
    strcpy(mensaje_nuevo, mensaje);
    strcat(mensaje_nuevo, IntToConstChar(suma, LARGO_FCS));
    cMessage * paquete_nuevo = new cMessage(mensaje_nuevo);
    return paquete_nuevo;
}

// Retorna verdadero si determina que "packet" no contiene errores
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

// Función utilizada para mostrar los mensajes del host
void enlace::VerMensajes(){
    ev << "Host " << host << endl;
    ev << "Mensajes: ( " << base << " )" << endl;
    for(int i = 0; i < mensajes_copia.size(); i++){
        ev << "\t" << ((cMessage*)mensajes_copia.at(i))->getFullName() << endl;
        if( i == base - 1){
            ev << "--------------------" << endl;
        }
    }
}

// Función utilizada para crear un acuse de recibo de la trama N°i.
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

// Función utilizada para crear una trama NACK de la trama N°i
cMessage * enlace::CrearNACK(int i, int origen, int destino){
    // Contenido del mensaje
    cMessage * ack = new cMessage((char*)IntToConstChar(i, LARGO_DATOS));
    // Mi Direccion
    ack = AnadirMensajeACMessage(ack, (char*)IntToConstChar(origen, LARGO_DIRECCION_ORIGEN), ANADIR_AL_INICIO);
    // Direccion de Envio
    ack = AnadirMensajeACMessage(ack, (char*)IntToConstChar(destino, LARGO_DIRECCION_DESTINO),ANADIR_AL_INICIO);
    // Tipo de mensaje (ack, datos, etc).
    ack = AnadirMensajeACMessage(ack, (char*)TRAMA_NACK, ANADIR_AL_INICIO);
    // Tipo de mensaje (token o datos)
    ack = AnadirMensajeACMessage(ack, (char*)CONTROL_ACCESO_TRAMA, LARGO_CONTROL_ACCESO);
    // Añado fcs a la trama
    ack = FCS(ack, SIN_ERROR);
    ack->setKind(COLOR_NACK);
    return ack;
}

// Retorna verdadero si "packet" es un ACK
bool enlace::EsAck(cMessage * packet){
    return strcmp(ObtenerMensaje(packet, INICIO_CONTROL_TRAMA, LARGO_CONTROL_TRAMA), TRAMA_ACK) == 0;
}

// Retorna verdadero si "packet" es un NACK
bool enlace::EsNack(cMessage * packet){
    return strcmp(ObtenerMensaje(packet, INICIO_CONTROL_TRAMA, LARGO_CONTROL_TRAMA), TRAMA_NACK) == 0;
}

// Retorna verdadero si "packet" es un mensaje
bool enlace::EsMensaje(cMessage * packet){
    return strcmp(ObtenerMensaje(packet, INICIO_CONTROL_TRAMA, LARGO_CONTROL_TRAMA), TRAMA_DATOS) == 0;
}

//lo que se hace cuando llega una palabra de codigo desde otro host
void enlace::processMsgFromLowerLayer(cMessage *packet)
{
    MensajeEnCamino = false;
    if(EsToken(packet)){ // si "packet" es el token
        TengoElToken = true;
        mensajes_enviados = 0;
        if(mensajes.size() > 0){
            EnviarMensaje();
            ev << "Host " << host << ":\n\t Tengo el token, y mensajes para enviar. Por lo tanto, transmitire." << endl;
        }else{
            ev << "Host " << host << ":\n\t Tengo el token, pero no tengo mensajes para enviar. Por lo tanto, enviare el token." << endl;
            EnviarToken();
            //send(packet, "hacia_fisico");
        }
    }else{ // si "packet" no es el token
        int direccion = par("direccion");
        // si la trama es para mi
        if(EsParaMi(packet, direccion)){
            if(EsAck(packet)){ // si la trama es ACK
                mensajes_confirmados++;
                ev << "Host " << host << ":\n\t Me llego un Acuse de Recibo del host " << ObtenerIntDeTrama(packet, INICIO_DIRECCION_ORIGEN, LARGO_DIRECCION_ORIGEN) << "..." << endl;
                mensajes_copia.pop_back();
                base--;
                //send(packet, "hacia_fisico");
                if(mensajes_enviados >= limite_de_mensajes || mensajes.size() == 0){
                    EnviarToken();
                }else{
                    EnviarMensaje();
                }
            }else if(EsNack(packet)){ // si la trama es NACK
                mensajes_confirmados++;
                ev << "Host " << host << ":\n\t Me llego un NACK del host " << ObtenerIntDeTrama(packet, INICIO_DIRECCION_ORIGEN, LARGO_DIRECCION_ORIGEN) << "..." << endl;
                mensajes_copia.pop_back();
                base--;
                //send(packet, "hacia_fisico");
                if(mensajes_enviados >= limite_de_mensajes || mensajes.size() == 0){
                    EnviarToken();
                }else{
                    EnviarMensaje();
                }
            }
            else if(EsMensaje(packet)){ // si la trama es un mensaje
                ev << "Host " << host << ":\n\t Me llego un mensaje de " << ObtenerIntDeTrama(packet, INICIO_DIRECCION_ORIGEN, LARGO_DIRECCION_ORIGEN) << ", mandare un Acuse de Recibo..." << endl;
                ev << "\tComprobando FCS... ";
                cMessage * paquete;
                if(ComprobarFcs(packet)){ // si la trama es valida
                    ev << "FCS Correcto. Trama valida." << endl;
                    paquete = CrearACK(ultimo_recibido, direccion, ObtenerIntDeTrama(packet, INICIO_DIRECCION_ORIGEN, LARGO_DIRECCION_ORIGEN));
                }else { // si la trama no es valida
                    ev << "FCS Incorrecto. Trama Invalida." << endl;
                    paquete = CrearNACK(ultimo_recibido, direccion, ObtenerIntDeTrama(packet, INICIO_DIRECCION_ORIGEN, LARGO_DIRECCION_ORIGEN));
                }

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
