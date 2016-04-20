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
	char *split, *directoryIndex; 
	char *method, *route, *version;
	char c;
	int i, s2, proceso, n, recibidos, max_clients=4, enviados;
	unsigned int long_client_addr;
	struct sockaddr_in server_addr, client_addr;
	char answer[1024], mensaje[1024],parameter[800], parameter1[800], parameter2[800], parameter3[800];
	FILE *conf_file, *asset;
	char *document_root; 
	int size;
	time_t tiempo;
	struct tm *tmPtr; 
	char date[80];
	char tamanyo[100];
	char *document;
	
	document_root=malloc(1024);
	document_root="/home/jose/Escritorio/Servidor";
	directoryIndex=malloc(1024);
	directoryIndex="/Indez.html";
	if(argc>1){
	  if(strcmp(argv[1], "-c")==0){
	    conf_file=fopen(argv[2], "r");
	   if(conf_file!=NULL){	
	      fgets(parameter1, 800, conf_file);
	      document_root=strtok(parameter1, " ");
	      fgets(parameter, 800, conf_file);
	      max_clients=atoi(parameter);
	      fgets(parameter2, 800, conf_file);
	      port_server=strtok(parameter2, " ");
	      fgets(parameter3, 800, conf_file);
	      directoryIndex=strtok(parameter3, " ");
	      fclose(conf_file);
	    }
	  }
	  else{
	    port_server=argv[1];
	    if(argc>2){
	      if(strcmp(argv[2], "-c")==0){
	    conf_file=fopen(argv[3], "r");
	   if(conf_file!=NULL){	
	      fgets(parameter1, 800, conf_file);
	      document_root=strtok(parameter1, " ");
	      fgets(parameter, 800, conf_file);
	      max_clients=atoi(parameter);
	      fgets(parameter2, 800, conf_file);
	      fgets(parameter3, 800, conf_file);
	      directoryIndex=strtok(parameter3, " ");
	      fclose(conf_file);
	    }
	  }
	    }
	  }	
	}
	
	
	/**** Paso 1: Abrir el socket ****/
	

	s = socket(AF_INET, SOCK_STREAM, 0); /* creo el socket */
	if (s == -1){
		fprintf(stderr, "Error. No se puede abrir el socket\n\r");
		return 1;
	}
	printf("Socket abierto\n\r");
	//printf("%s\n",document_root);
	//printf("%d\n",max_clients);
	//printf("%s\n",port_server);
	//printf("%s\n",directoryIndex);
	
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
	
	//printf("%s\n",document_root);
	//printf("%s\n",directoryIndex);
	
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
				strcat(answer,"HTTP/1.1 500 Internal Server Error\n");
					strcat(answer, "Connection: close\n\r");
					strcat(answer, "Content-Length: 96");
					strcat(answer, "\n\r");
					strcat(answer, "Content-Type: txt/html\n\r");
					strcat(answer, "Server: Servidor SD\n\r");
					strcat(answer, "Date: ");
					strcat(answer, date);
					strcat(answer, "\n\r");
					strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
					strcat(answer, "\n\r");
					strcat(answer, "<html> <title>Error 500</title>\n<h1> Error 500: Error Interno. </h1> </html>");
					n = strlen(answer);
				enviados = write(s2, answer, n);
				if (enviados == -1 || enviados < n)
				{
					fprintf(stderr, "Error enviando la respuesta (%d)\n\r",enviados);
					close(s);
					return 1;
				}

			close(s2);
			exit(0); /* el hijo ya no tiene que hacer nada */			}
			mensaje[recibidos] = '\0'; /* pongo el final de cadena */
			printf("Mensaje [%d]: %s\n\r", recibidos, mensaje); /*Para que mostramos esta linea?*/
			method=strtok(mensaje, " "); /* Comprobamos el metodo HTTP*/
			route=strtok(NULL, " ");
			version=strtok(NULL," ");
			tiempo = time(NULL);
			tmPtr = localtime(&tiempo);
			strftime(date, 80, "%H:%M:%S, %A de %B de %Y", tmPtr);
		 
			if(strcmp(method, "GET")==0){
				strcat(document_root, route);
				//printf("%s\n",document_root);
				     //printf("%s\n",route);
				     //printf("%s\n",version);
				if(strcmp(version,"HTTP/1.1")>=0){
					asset=fopen(document_root, "r");					
					if(asset==NULL){
						strcpy(answer, "HTTP/1.1 404 not found\n\r");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 143");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer, "\n\r"); 
						strcat(answer, "<html> <title> Error 404</title>\n<h1> Error 404: Archivo no encontrado en el servidor  </h1> \n O a lo mejor no queriamos que lo encontraras... </html>");
					}
					else{				 
						strcat(answer, "HTTP/1.1 200 OK\n\r");
						fseek(asset,0L,SEEK_END);
						size=ftell(asset);
						sprintf(tamanyo,"%d",size);
						fseek(asset,0L,SEEK_SET);
						document=malloc(size);
						if(document){
						  fread(document,1,size,asset);
						}
						fclose(asset);
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: ");
						strcat(answer,tamanyo);
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer,"\n\r");
						
						if(document){
						  strcat(answer,document);
						}
						strcat(answer, "\n\r");
						
					}
				}

				else{
				  
					strcat(answer,"HTTP/1.1 505 HTTP version not supported\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 90");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer, "\n\r");
						strcat(answer, "<html> <title>Error 505</title>\n<h1> Error 505: Version De HTTP no soportada. </h1> </html>");
				}
			}
			
			else if(strcmp(method, "HEAD")==0){
				strcat(document_root, route);
				asset=fopen(document_root, "r");
				if(strcmp(version,"HTTP/1.1")>=0){
					if(asset==NULL){
						strcpy(answer, "HTTP/1.1 404 not found\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 143");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer, "\n");
						strcat(answer, "<html> <title> Error 404</title>\n<h1> Error 404: Archivo no encontrado en el servidor  </h1> \n O a lo mejor no queriamos que lo encontraras... </html>");
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
					strcat(answer,"HTTP/1.1 505 HTTP version not supported\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 90");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer, "<html> <title>  Error 505 </title>\n<h1> Error 505: Version De HTTP no soportada. </h1> </html>");				}
				if (asset!=NULL){ 
					fclose(asset);
				}
			}
			
			else if(strcmp(method, "PUT")==0){
				strcat(document_root, route);
				
				/*Operamos para el metodo PUT*/
				if(strcmp(version,"HTTP/1.1")>=0){
					asset=fopen(document_root, "w");
					
					if(asset==NULL){
						strcat(answer,"HTTP/1.1 403 Forbidden\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 77");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer, "\n");
						strcat(answer, "<html> <title>Error 403</title>\n<h1> Error 403: Acceso Denegado. </h1> </html>");
						
					}else{
						strcpy(answer, "HTTP/1.1 201 CREATED\n");
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
					

				}else{
					strcat(answer,"HTTP/1.1 505 HTTP version not supported\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 90");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer, "<html> <title>  Error 505 </title>\n<h1> Error 505: Version De HTTP no soportada. </h1> </html>");				}
				if (asset!=NULL){
					fclose(asset);
				}
			}
			
			else if(strcmp(method, "DELETE")==0){
				int aux;
				aux=-1;
				char name[strlen(route)];
				strcat(document_root, route);
				/*Operamos para el metodo DELETE*/
				if(strcmp(version,"HTTP/1.1")>=0){
					aux=remove(document_root);
					if(aux!=0){
						strcat(answer,"HTTP/1.1 404 not found\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 143");
						strcat(answer, "\n\r");
						strcat(answer, "Content-Type: txt/html\n\r");
						strcat(answer, "Server: Servidor SD\n\r");
						strcat(answer, "Date: ");
						strcat(answer, date);
						strcat(answer, "\n\r");
						strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
						strcat(answer, "<html> <title> Error 404</title>\n<h1> Error 404: Archivo no encontrado en el servidor  </h1> \n O a lo mejor no queriamos que lo encontraras... </html>");
					}else{
						strcat(answer,"HTTP/1.1 200 OK\n");
						strcat(answer, "Connection: close\n\r");
						strcat(answer, "Content-Length: 0");//Debemos dar un index html para put y delete!
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
					strcat(answer, "Content-Length: 90");
					strcat(answer, "\n\r");
					strcat(answer, "Content-Type: txt/html\n\r");
					strcat(answer, "Server: Servidor SD\n\r");
					strcat(answer, "Date: ");
					strcat(answer, date);
					strcat(answer, "\n\r");
					strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
					strcat(answer, "<html> <title>  Error 505 </title>\n<h1> Error 505: Version De HTTP no soportada. </h1> </html>");
				}	
			}else if(strcmp(method,"OPTIONS")==0 || strcmp(method,"POST")==0 || strcmp(method,"TRACE")==0 || strcmp(method,"CONNECT")==0){
				strcpy(answer, "HTTP/1.1 400 bad request\n");
				strcat(answer, "Connection: close\n\r");
				strcat(answer, "Content-Length: 93");
				strcat(answer, "\n\r");
				strcat(answer, "Content-Type: txt/html\n\r");
				strcat(answer, "Server: Servidor SD\n\r");
				strcat(answer, "Date: ");
				strcat(answer, date);
				strcat(answer, "\n\r");
				strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
				strcat(answer,"<html> <title>  Error 400 </title>\n<h1> Error 400: Petición errónea. </h1> </html>");
			}
			else{
				strcpy(answer, "HTTP/1.1 405 method not allowed\n");
				strcat(answer, "Connection: close\n\r");
				strcat(answer, "Content-Length: 82");
				strcat(answer, "\n\r");
				strcat(answer, "Content-Type: txt/html\n\r");
				strcat(answer, "Server: Servidor SD\n\r");
				strcat(answer, "Date: ");
				strcat(answer, date);
				strcat(answer, "\n\r");
				strcat(answer, "Cache-control: max-age=0, no-cache\n\r");
				strcat(answer,"<html> <title>  Error 405 </title>\n<h1> Error 405: Método no permitido. </h1> </html>");
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
