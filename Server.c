#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h> 

int s;

void finalizar (int senyal){
	printf("Recibida la señal de fin (cntr-C)\n\r");
	close(s); /* cerrar para que accept termine con un error y salir del bucle principal */
}

int main(int argc, char *argv[]){
	char *port_server="6000";
	char *split, *directoryIndex; /*Debe indicar directoryIndex un documento por defecto ya presente en el servidor? Como tratamos errores*/
	char *method, *route;
	char c;
	int i, s2, proceso, n, recibidos, max_clients=4;
	unsigned int long_client_addr;
	struct sockaddr_in server_addr, client_addr;
	char answer[1024], mensaje[1024], parameter[200];
	char *document_root; /*Esto es un puntero? O deberia ser un array?*/
	
	/*Esto solo tiene sentido si es obligatorio introducir un puerto */
	if (argc < 2){
		fprintf(stderr, "Error. Debe indicar el puerto del servidor\r\n");
		fprintf(stderr, "Sintaxis: %s <puerto>\n\r", argv[0]);
		fprintf(stderr, "Ejemplo : %s 8574\"\n\r", argv[0]);
		return 1;
	}
	
	for(i=1; i<argc; i++){
		if(strcmp(argv[i],"-c")==0){ /*Faltan mas comprobaciones? */
			i++;
			if(i<argc){
				conf_file=fopen(argv[i], "r");  /*Podemos crear un fichero de configuracion donde sea obligatorio pasar todos los parametros?*/
				if(fgets(parameter, 200, conf_file)!=null){
					strcpy(document_root, parameter);
					if(fgets(parameter, 200, conf_file)!=null){
						max_clients=atoi(parameter);
						if(fgets(parameter, 200, conf_file)!=null){ /*Tiene sentido indicarlo aqui y por linea de comandos?*/
							if(strcmp(argv[1], "-c")==0){
								strcpy(port_server, parameter);
							}
							if(fgets(parameter, 200, conf_file)!=null){
								strcpy(directoryIndex, parameter);
							}
						}
					}
				}
				fclose(conf_file);
			}
		}
		else if(i==1){
			strcpy(port_server, argv[i]); /*Podriamos comprobar que sea >1024*/
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
	server_addr.sin_port = htons(atoi(port_server));
	server_addr.sin_addr.s_addr = INADDR_ANY; /* cualquier IP del servidor */
	if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		fprintf(stderr, "Error. No se puede asociar el puerto al servidor\n\r");
		close(s);
		return 1;
	}
	printf("Puerto de escucha establecido\n\r");
	
	/**** Paso 3: Preparar el servidor para escuchar ****/

	if (listen(s, max_clients) == -1){
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
			if (recibidos == -1){
				fprintf(stderr, "Error leyendo el mensaje\n\r");
				exit(1);
			}
			mensaje[recibidos] = '\0'; /* pongo el final de cadena */
			printf("Mensaje [%d]: %s\n\r", recibidos, mensaje); /*Para que mostramos esta linea?*/
			/*A partir de aqui interpretamos la cabecera*/
			method=strtok(mensaje, " "); /* Comprobamos el metodo HTTP*/
			if(strcmp(method, "GET")==0){
				route=strtok(NULL, " ");
				strcat(document_root, route); /* Falta una /?? */
				asset=fopen(document_root, "r");
				if(asset==NULL){
					strcpy(answer, "HTTP1.1 404 not found\n");
					/*Cabeceras*/
					strcat(answer, "\n");
				}
				else{
					strcpy(answer, "HTTP1.1 200 OK\n");
					/*Cabeceras*/
					while(c=getc(asset)!=EOF){
						strcat(answer, c);
					}
					strcat(answer, "\0");
				}
			}
			else if(strcmp(method, "HEAD")==0){
				route=strtok(NULL, " ");
				strcat(document_root, route);
				asset=fopen(document_root, "r");
				if(asset==NULL){
					strcpy(answer, "HTTP1.1 404 not found\n");
					/*Cabeceras*/
					strcat(answer, "\n");
				}
				else{
					strcpy(answer, "HTTP1.1 200 OK\n");
					/*Cabeceras*/
					strcat(answer, "\n");
				}
			}
			else if(strcmp(method, "PUT")==0){
				route=strtok(NULL, " ");
				strcat(document_root, route);
				/*Operamos para el metodo PUT*/
			}
			else if(strcmp(method, "DELETE")==0){
				route=strtok(NULL, " ");
				strcat(document_root, route);
				/*Operamos para el metodo DELETE*/
			}
			else{
				strcpy(answer, "HTTP1.1 405 method not allowed\n");
				/*Hacemos un HTTP o sacamos stderr?*/
			}
			
		}
		else{ /* soy el padre */
			close(s2); /* el padre no usa esta conexión */
		}
	}
}