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
				conf_file=argv[i];
				/*Tratamos aqui dentro el fichero de configuracion? Como?*/
			}
		}
		else if(i==1){
			port_server=atoi(argv[i]);
		}
	}
	
	
}