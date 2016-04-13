//Mariano López Escudero
//Alejandro Martínez Martínez
//Adrián Francés Lillo

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
	char *method, *route, *version;
	char c;
	int i, s2, proceso, n, recibidos, max_clients=4, enviados;
	unsigned int long_client_addr;
	struct sockaddr_in server_addr, client_addr;
	char answer[1024], mensaje[1024], parameter[200];
	FILE *conf_file, *asset;
	char *document_root; /*Esto es un puntero? O deberia ser un array?*/
	long int size;
	time_t time;
	struct tm *tmPtr; 
	char date[80];
	
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
				conf_file=fopen(argv[i], "r");
				if(conf_file!=NULL){
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
				else{
					fprintf(stderr, "Error. No se ha podido abrir el fichero de configuración");
				}
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
			method=strtok(mensaje, " "); /* Comprobamos el metodo HTTP*/
			route=strtok(NULL, " ");
			version=strtok(NULL," ");
			time = time(NULL);
			tmPtr = localtime(&time);
			strftime(date, 80, "%H:%M:%S, %A de %B de %Y", tmPtr);
			if(strcmp(method, "GET")==0){
				strcat(document_root, route);
				if(strcmp(version,"HTTP/1.1")==0){ 
					asset=fopen(document_root, "r");
					if(asset==NULL){
						strcpy(answer, "HTTP/1.1 404 not found\n\r");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 0");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						/*Cabeceras*/
						strcat(answer, "\n\r"); //Es necesario?
					}
					else{
						strcpy(answer, "HTTP/1.1 200 OK\n\r");
						fseek(asset,0L,SEEK_END);
						size=ftell(asset);
						fseek(asset,0L,SEEK_SET);
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: ");
						strcat(answer, (char)size); /*No se si funcionará el casteo*/
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						
						
						while(c=getc(asset)!=EOF){
							strcat(answer, c);
						}
						strcat(answer, "\0");
						
					}

				}else{
					strcat(answer,"505 HTTP version not supported\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 0");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
				}
				if (asset!=NULL){ 
					fclose(asset);
				}
			}
			
			else if(strcmp(method, "HEAD")==0){
				strcat(document_root, route);
				asset=fopen(document_root, "r");
				if(strcmp(version,"HTTP/1.1")==0){ //anyadido Alejandro
					if(asset==NULL){
						strcpy(answer, "HTTP/1.1 404 not found\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 0");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer, "\n");
						
					}
					else{
						strcpy(answer, "HTTP/1.1 200 OK\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 0");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type:\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer, "\n");
					}

					
				}else{
					strcat(answer,"505 HTTP version not supported\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 0");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
				}
				if (asset!=NULL){ 
					fclose(asset);
				}
			}
			
			else if(strcmp(method, "PUT")==0){
				strcat(document_root, route);
				asset=fopen(document_root, "w");
				/*Operamos para el metodo PUT*/
				if(strcmp(version,"HTTP/1.1")==0){ 
					if(asset==NULL){
						strcat(answer,"403 Forbidden\n");
					}else{
						strcpy(answer, "HTTP/1.1 201 CREATED\n");

					}
					

				}else{
					strcat(answer,"505 HTTP version not supported\n");
					/*Cabeceras???*/
				}
				if (asset!=NULL){ //anyadido Alejandro
					fclose(assetPUT);
				}
			}
			
			else if(strcmp(method, "DELETE")==0){
				int aux;
				aux=-1;
				char name[strlen(route)];
				strcat(document_root, route);
				/*Operamos para el metodo DELETE*/
				if(strcmp(version,"HTTP/1.1")==0){ //anyadido Alejandro
					aux=remove(name);
					if(aux!=0){
						strcat(answer,"HTTP/1.1 404 not found\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: ");
						strcat(answer, (char)size); /*No se si funcionará el casteo*/
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
					}else{
						strcat(answer,"HTTP/1.1 200 OK\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: ");
						strcat(answer, (char)size); /*No se si funcionará el casteo*/
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
					}
				}else{
					strcat(answer,"HTTP/1.1 505 HTTP version not supported\n");
					strcat(answer, "Connection: close\n\r");
					strcat(answer, "Content-Length: ");
					strcat(answer, (char)size); /*No se si funcionará el casteo*/
					strcat(answer, "\n\r");
					strcat(answer, "Content-Type: txt/html\n\r");
					strcat(answer, "Server: Servidor SD\n\r");
					strcat(answer, "Date: ");
					strcat(answer, date);
					strcat(answer, "\n\r");
					strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
				}	
			}
			else{
				strcpy(answer, "HTTP/1.1 405 method not allowed\n");
				strcat(answer, "Connection: close\n\r");
				strcat(answer, "Content-Length: ");
				strcat(answer, (char)size); /*No se si funcionará el casteo*/
				strcat(answer, "\n\r");
				strcat(answer, "Content-Type: txt/html\n\r");
				strcat(answer, "Server: Servidor SD\n\r");
				strcat(answer, "Date: ");
				strcat(answer, date);
				strcat(answer, "\n\r");
				strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
			}
			
			/**** Paso 6: Enviar respuesta ****/
			n = strlen(answer);
			enviados = write(s2, answer, n);
			if (enviados == -1 || enviados < n)
			{
				fprintf(stderr, "Error enviando la respuesta (%d)\n\r",enviados);
				close(s);
				return 1;
			}

			close(s2);
			exit(0); /* el hijo ya no tiene que hacer nada */
		}
		else{ /* soy el padre */
			close(s2); /* el padre no usa esta conexión */
		}
	}
}