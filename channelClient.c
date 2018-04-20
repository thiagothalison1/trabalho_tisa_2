#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include "circularList.h"
#include "channelMessageProtocol.h"

#define FALHA 1

/**
 * Global Variables
 */
int socket_local;
char last_received_seq_number = (char) 255;
struct sockaddr_in endereco_destino;
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;

int cria_socket_local(void) {
	socket_local = socket( PF_INET, SOCK_DGRAM, 0);
	if (socket_local < 0) {
		perror("socket");
		return 1;
	}
	return 0;
}

struct sockaddr_in cria_endereco_destino(char *destino, int porta_destino) {
	struct sockaddr_in servidor; 	/* Endereço do servidor incluindo ip e porta */
	struct hostent *dest_internet;	/* Endereço destino em formato próprio */
	struct in_addr dest_ip;		    /* Endereço destino em formato ip numérico */

	if (inet_aton ( destino, &dest_ip ))
		dest_internet = gethostbyaddr((char *)&dest_ip, sizeof(dest_ip), AF_INET);
	else
		dest_internet = gethostbyname(destino);

	if (dest_internet == NULL) {
		fprintf(stderr,"Endereço de rede inválido\n");
		exit(FALHA);
	}

	memset((char *) &servidor, 0, sizeof(servidor));
	memcpy(&servidor.sin_addr, dest_internet->h_addr_list[0], sizeof(servidor.sin_addr));
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(porta_destino);

	return servidor;
}

int recebe(char *buffer) {
	int bytes_recebidos;		/* Número de bytes recebidos */

	/* Espera pela msg de resposta do servidor */
	bytes_recebidos = recvfrom(socket_local, buffer, CHANNEL_PACKAGE_SIZE, 0, NULL, 0);
	if (bytes_recebidos < 0)
	{
		perror("recvfrom");
	}

	return bytes_recebidos;
}

void dispatch(char seqNumber, char messageType, char message) {
    char package[CHANNEL_PACKAGE_SIZE];

    buildChannelPackage(messageType, seqNumber, message, package);

	/* Envia msg ao servidor */
	if (sendto(socket_local, package, strlen(package)+1, 0, (struct sockaddr *) &endereco_destino, sizeof(endereco_destino)) < 0 ) { 
		perror("sendto");
		return;
	}
}

void sendMessage(char seqNumber, char messageType, char message) {
	dispatch(seqNumber, messageType, message);
}

void openConnection(char *host, int port) {
	cria_socket_local();
	endereco_destino = cria_endereco_destino(host, port);
}

void startClient () {
	char *host = "localhost";
	int port = 8888;
	openConnection(host, port);

	sendMessage(DUMMY_SEQ_NUMBER, CONNECT_MSG, DUMMY_MESSAGE);
}

void listenServer () {
	while (1) {
		char serverPackage[CHANNEL_PACKAGE_SIZE];

		recebe(serverPackage);

		struct channelMessage serverMessage;

		int status = parseChannelPackage(serverPackage, &serverMessage);

		if (status == 1) {
			if (serverMessage.seqNumber != last_received_seq_number) {
				insert(serverMessage.message);
				last_received_seq_number = serverMessage.seqNumber;
			}
			sendMessage(serverMessage.seqNumber, ACK_MSG, DUMMY_MESSAGE);
		} else {
			sendMessage(serverMessage.seqNumber, NACK_MSG, DUMMY_MESSAGE);
		}
	}
}
