import
    "host";

module sistema
    parameters:
        tamTrama: numeric;

    submodules:

        Host_0: host;
            parameters:
                tamTrama = tamTrama,
                direccion = 0;

            display: "p=71,79;i=old/server1";
        Host_1: host;
            parameters:
                tamTrama = tamTrama,
                direccion = 1;

            display: "p=71,179;i=old/server1";
        Host_2: host;
            parameters:
                tamTrama = tamTrama,
                direccion = 2;

            display: "p=190,179;i=old/server1";
        Host_3: host;
            parameters:
                tamTrama = tamTrama,
                direccion = 3;

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
        tamTrama = input(4, "Ingrese el largo de la palabra, entero mayor a cero:");
endnetwork

