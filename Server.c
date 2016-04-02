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

int main(int argc, char *argv[]){
	char *port_server="6000";
	char *conf_file;
	int i;
	struct sockaddr_in server_addr;
	
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
	if (s == -1)
	{
		fprintf(stderr, "Error. No se puede abrir el socket\n\r");
		return 1;
	}
	printf("Socket abierto\n\r");
	
	/**** Paso 2: Establecer la dirección (puerto) de escucha ****/

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(servidor_puerto));
	server_addr.sin_addr.s_addr = INADDR_ANY; /* cualquier IP del servidor */
	if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		fprintf(stderr, "Error. No se puede asociar el puerto al servidor\n\r");
		close(s);
		return 1;
	}
	printf("Puerto de escucha establecido\n\r");
	
	/**** Paso 3: Preparar el servidor para escuchar ****/

	if (listen(s, 4) == -1) /*Solo vamos a escuchar 4 simultaneos? Deberiamos confirmar la cantidad*/
	{
		fprintf(stderr, "Error preparando servidor\n\r");
		close(s);
		return 1;
	}
	printf("Socket preparado\n\r");
}