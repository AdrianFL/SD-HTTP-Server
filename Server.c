#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h> /*Aun no se si esta hace falta*/

int s;

void finalizar (int senyal){
	printf("Recibida la señal de fin (cntr-C)\n\r");
	close(s); /* cerrar para que accept termine con un error y salir del bucle principal */
}

int main(int argc, char *argv[]){
	char *port_server="6000";
	char *conf_file;
	int i, s2, proceso, n;
	unsigned int long_client_addr;
	struct sockaddr_in server_addr, client_addr;
	char respuesta[1024];
	
	/*Esto solo tiene sentido si es obligatorio introducir un puerto */
	if (argc < 2){
		fprintf(stderr, "Error. Debe indicar el puerto del servidor\r\n");
		fprintf(stderr, "Sintaxis: %s <puerto>\n\r", argv[0]);
		fprintf(stderr, "Ejemplo : %s 8574\"\n\r", argv[0]);
		return 1;
	}
	
	for(i=0; i<argc; i++){
		if(strcmp(argv[i],"-c")==0){ /*Faltan mas comprobaciones? */
			i++;
			if(i<argc){
				conf_file=argv[i]; /*Puede que necesitemos strcpy*/
				/*Tratamos aqui dentro el fichero de configuracion? Como?*/
			}
		}
		else if(i==1){
			port_server=atoi(argv[i]); /*Podriamos comprobar que sea >1024*/
		}
	}
	
	/**** Paso 1: Abrir el socket ****/

	s = socket(AF_INET, SOCK_STREAM, 0); /* creo el socket */
	if (s == -1){
		fprintf(stderr, "Error. No se puede abrir el socket\n\r");
		return 1;
	}
	printf("Socket abierto\n\r");
	
	/**** Paso 2: Establecer la dirección (puerto) de escucha ****/

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(servidor_puerto));
	server_addr.sin_addr.s_addr = INADDR_ANY; /* cualquier IP del servidor */
	if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		fprintf(stderr, "Error. No se puede asociar el puerto al servidor\n\r");
		close(s);
		return 1;
	}
	printf("Puerto de escucha establecido\n\r");
	
	/**** Paso 3: Preparar el servidor para escuchar ****/

	if (listen(s, 4) == -1){ /*Solo vamos a escuchar 4 simultaneos? Deberiamos confirmar la cantidad*/
		fprintf(stderr, "Error preparando servidor\n\r");
		close(s);
		return 1;
	}
	printf("Socket preparado\n\r");
	
	/**** Paso 4: Esperar conexiones ****/

	signal(SIGINT, finalizar);

	while (1){
		fprintf(stderr, "Esperando conexión en el puerto %s...\n\r", port_server);
		long_client_addr = sizeof (client_addr); /* Por que lo declara aparte en lugar de hacer un sizeof directamente?*/
		s2 = accept (s, (struct sockaddr *)&client_addr, &long_client_addr);
		/* s2 es el socket para comunicarse con el cliente */
		/* s puede seguir siendo usado para comunicarse con otros clientes */
		if (s2 == -1)
		{
			break; /* salir del bucle */
		}
		/* crear un nuevo proceso para que se pueda atender varias peticiones en paralelo */
		proceso = fork();
		if (proceso == -1) exit(1);
		if (proceso == 0){ /* soy el hijo */
			close(s); /* el hijo no necesita el socket general */

			/**** Paso 5: Leer el mensaje ****/

			n = sizeof(mensaje);
			recibidos = read(s2, mensaje, n);
			/*A partir de aqui interpretamos la cabecera*/
		}
		else{ /* soy el padre */
			close(s2); /* el padre no usa esta conexión */
		}
	}
}