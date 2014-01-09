import
    "host";

module sistema
	parameters:
	    cantidad_de_mensajes: numeric;
    submodules:

        Host_0: host;
            parameters:
                cantidad_de_mensajes = cantidad_de_mensajes,
                direccion = 0,
                direccion_de_envio = 1;
                

            display: "p=71,79;i=old/server1";
        Host_1: host;
            parameters:
                direccion = 1,
                cantidad_de_mensajes = cantidad_de_mensajes,
                direccion_de_envio = 2;

            display: "p=71,179;i=old/server1";
        Host_2: host;
            parameters:
                direccion = 2,
                cantidad_de_mensajes = cantidad_de_mensajes,
                direccion_de_envio = 3;

            display: "p=190,179;i=old/server1";
        Host_3: host;
            parameters:
                direccion = 3,
                cantidad_de_mensajes = cantidad_de_mensajes,
                direccion_de_envio = 0;

            display: "p=190,79;i=old/server1";
    connections:
        Host_0.salida --> canal --> Host_1.entrada;

        Host_1.salida --> canal --> Host_2.entrada;

        Host_2.salida --> canal --> Host_3.entrada;

        Host_3.salida --> canal --> Host_0.entrada;
    display: "b=260,242";
endmodule

channel canal
    delay 100ms;
    datarate 128000bps;
   // error 1e-8;
endchannel

//INSTANCIACION DE LA RED
network red : sistema
	parameters:
        cantidad_de_mensajes = input(2, "Ingrese la cantidad de mensajes que enviara cada host:");    
endnetwork

