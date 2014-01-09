import
    "enlace",
    "intermedio",
    "aplicacion";

module host
    parameters:
        cantidad_de_mensajes: numeric const,
        direccion: numeric const,
        direccion_de_envio: numeric const;

    gates:
        in: entrada;
        out: salida;

    submodules:

        varAplicacion: aplicacion;
            parameters:
                direccion_de_envio = direccion_de_envio,
                cantidad_de_mensajes = cantidad_de_mensajes,                
                direccion = direccion;

            display: "p=98,44;i=abstract/penguin";
            
        varIntermedio: intermedio;
        	parameters:
	            display: "p=98,152;i=abstract/switch";
	            
        varEnlace: enlace;
            parameters:
                limite_de_tramas = 3,
                direccion_de_envio = direccion_de_envio,
                direccion = direccion;

            display: "p=97,245;i=block/plug";
    connections:
        varAplicacion.hacia_abajo --> varIntermedio.desde_arriba;
        varIntermedio.hacia_arriba --> varAplicacion.desde_abajo;
        varIntermedio.hacia_abajo --> varEnlace.desde_arriba;
        varEnlace.hacia_arriba --> varIntermedio.desde_abajo;
        varEnlace.hacia_fisico --> salida;
        entrada --> varEnlace.desde_fisico;

    display: "o=,,;b=208,315,,,,";
endmodule

