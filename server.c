#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <openssl/md5.h>

#define PORT 5454

char buffer[1024];

char server_h[] = "2425d371eb5cddcf70a683821e9026ab32c26c82";
char client_h[] = "96f846434346e6eca593c7b179b5723a6d6a242a";

typedef struct {
	int socket, flag;
	char ip[15];
	char name[32];
}server_t;


server_t servers[20];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*

void send_message(client* cli, char* buf){
        pthread_mutex_lock(&mutex);
                send(cli->sockfd, buf, strlen(buf), 0);
        pthread_mutex_unlock(&mutex);
}
void broadcast(client* cli, char* buf){
        pthread_mutex_lock(&mutex);
        for(int i=0;i<20;i++){
                if(!(strcmp(clients[i].name, cli->name)==0) && clients[i].flag){
                        send(clients[i].sockfd, buf, strlen(buf),0);
                }
        }
        pthread_mutex_unlock(&mutex);
}


void *server(void *arg){
	*server_t = (server_t*)arg;
	
	
	pthread_detach(pthread_self());
}

void *client(void *arg){
	int sockfd = *((int*)arg);
	


	pthread_detach(pthread_self());
}
*/



void serializeServer(server_t *server, char *buffer){
	memcpy(buffer, server->name, 32);
	buffer += 32;
	memcpy(buffer, server->ip, 15);
}

int main(){


        int server_fd, client_socket;
        struct sockaddr_in serv_address;
        struct sockaddr_in cli_address;
        int opt = 1;
        //set up what socket the server will be listening on
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        //set the socket options to reuse the port because it would be annoying asf without
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

        //set up socket scheme
        serv_address.sin_family = AF_INET;
        serv_address.sin_addr.s_addr = INADDR_ANY;
        serv_address.sin_port = htons(PORT);

        //set that socket the server will be listening on up by connecting it with the addresses it will listen and port on
        if(bind(server_fd, (struct sockaddr*)&serv_address, sizeof(serv_address)) < 0);

        //now tell the kernel we want to listen for connections
        listen(server_fd, 20);

        printf("Server up\n");
        socklen_t clilen = sizeof(cli_address);
        while(1){
                //now wait for a connection and if that connection comes in let a seperate thread process handle it until it ends
                client_socket = accept(server_fd, (struct sockaddr*)&cli_address, &clilen);
                //if a client is accepted we let it get handled by its own thread process

		pthread_t handleid;
		char *ip;

		int leave;

                printf("connection recieved: %d\n",client_socket);
        	recv(client_socket, buffer, 20, 0);
		server_t* current;
		if(memcmp(buffer, server_h, 20)==0){
			//after this statement it just fuckign stops why the fuck actually why the fuck
			
			recv(client_socket, &leave, 4, 0);
			if(leave == 1){
				bzero(buffer, 20);
				recv(client_socket, buffer, 32 ,0);
				for(int i=0;i<20;i++){
					if(servers[i].flag){
						if(strcmp(servers[i].name, buffer)==0){
							printf("server %s with ip address %s has left and unregistered\n",servers[i].name, servers[i].ip);
							servers[i].flag = 0;
							close(client_socket);
						}
					}
				}
			}
			else {
				printf("Server Connect\n");
				for(int i=0;i<20;i++){
					//printf("Server Connected\n");
					if(!servers[i].flag){
						servers[i].flag = 1;
						bzero(buffer, 20);
						recv(client_socket, buffer, 32, 0);
						strcpy(servers[i].name, buffer);
						servers[i].socket = client_socket;
						bzero(buffer, 32);
						recv(client_socket, buffer, 15, 0);
						strcpy(servers[i].ip, buffer);
						current = &servers[i];
						printf("server connected ip: %s server name: %s\n", current->ip, current->name);
						close(client_socket);
						break;
					}		

				}
			}			
		}        
//		int size;
		else if(memcmp(buffer, client_h, 20)==0){
			printf("Client Connected\n");
			for(int i=0;i<20;i++){
				if(servers[i].flag){
					printf("sending\n");
					bzero(buffer, 1024);
					serializeServer(&servers[i], buffer);
					send(client_socket, buffer, (32+15), 0);
				}	
			}
			close(client_socket);
		}
        }
        return 0;
}
