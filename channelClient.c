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
#include "timestamp.h"
#include "circularList.h"
#include "channelMessageProtocol.h"

/*** Constants ***/

#define FALHA_ENDERECO_DE_REDE 1 // Utilizado como identificador de falhas na criação de socket com o servidor de dados.

/*** Global Variables ***/

int localSocket; // Armazena o identificador do socket com o servidor.
char lastReceivedSeqNumber = (char) 255; // Armazena o número do último número de sequência de pacote recebido.
struct sockaddr_in serverAddress; // Armazena os dados referentes ao endereço do servidor.
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;

/*** Functions ***/

/**
* Esta função é responsável pela criação do socket de comunicação com o servidor.
*/
int createServerSocket(void) {
	localSocket = socket( PF_INET, SOCK_DGRAM, 0);
	if (localSocket < 0) {
		perror("socket");
		return 1;
	}
	return 0;
}

/**
* Esta função é responsável pela criação de uma estrutura de dados que armazena as informações 
* referentes ao endereço do servidor endereço e porta.
* Parâmetros: char data: dado
*             struct timeval * timestamp: dados de tempo da mensagem
*/
struct sockaddr_in createServerAddress(char *address, int port) {
	struct sockaddr_in servidor; 	// Endereço do servidor incluindo ip e porta
	struct hostent *dest_internet;	// Endereço destino em formato próprio
	struct in_addr dest_ip;		    // Endereço destino em formato ip numérico

	if (inet_aton(address, &dest_ip))
		dest_internet = gethostbyaddr((char *)&dest_ip, sizeof(dest_ip), AF_INET);
	else
		dest_internet = gethostbyname(address);

	if (dest_internet == NULL) {
		fprintf(stderr,"Endereço de rede inválido\n");
		exit(FALHA_ENDERECO_DE_REDE);
	}

	memset((char *) &servidor, 0, sizeof(servidor));
	memcpy(&servidor.sin_addr, dest_internet->h_addr_list[0], sizeof(servidor.sin_addr));
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(port);

	return servidor;
}

/**
* Esta função é responsável pela criação de uma estrutura de dados que armazena as informações 
* referentes ao endereço do servidor endereço e porta.
* Parâmetros: char buffer: buffer para armazenamento de dados recebidos do servidor.
*/
int receiveData(char *buffer) {
	int receivedBytes; // Número de bytes recebidos

	// Espera por alguma mensagem do servidor.
	receivedBytes = recvfrom(localSocket, buffer, CHANNEL_PACKAGE_SIZE, 0, NULL, 0);
	if (receivedBytes < 0)
	{
		perror("recvfrom");
	}

	return receivedBytes;
}

/**
* Esta função é responsável pelo envio de pacotes para o servidor.
* Parâmetros: char seqNumber: Número do pacote a ser envidado.
*             char messageType: Tipo de mensagem a ser enviada. Neste caso mensagem de dados.
*			  char message: A mensagem a ser enviada.
*/
void sendMessage(char seqNumber, char messageType, char message) {
	char package[CHANNEL_PACKAGE_SIZE];

	// Recupera o horário local para utilizar como timestamp da mensagem.
	struct timeval timeNow;
	getTime(&timeNow);

	// Coloca as informações referentes ao pacote no buffer (package)
    buildChannelPackage(messageType, seqNumber, message, &timeNow, package);

	// Envia o pacote para o servidor
	if (sendto(localSocket, package, CHANNEL_PACKAGE_SIZE + 1, 0, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0 ) { 
		perror("sendto");
		return;
	}
}

/**
* Esta função é responsável pelo setUp da comunicação com o servidor.
* Ela chama uma função que cria o socket de comunicação e outra que configura o endereço de envio dos dados.
* Parâmetros: char host: Endereço do servidor.
*             int port: Porta do servidor.
*/
void openConnection(char *host, int port) {
	createServerSocket();
	serverAddress = createServerAddress(host, port);
}

/**
* Esta função é responsável por iniciar a comunicação com o servidor. Para tal ela abre a comunicação
* e envia uma mensagem de abertura de conexão. 
* A mensagem de abertura de conexão não tem dadados e nem número de sequência na primeira versão do channel.
*/
void startClient () {
	char *host = "localhost";
	int port = 8888;

	openConnection(host, port);
	sendMessage(DUMMY_SEQ_NUMBER, CONNECT_MSG, DUMMY_MESSAGE);
}

/**
* Esta função implementa um loop que fica constantemente "escutando" por mensagens do servidor.
* Ela controla o número de sequência das mensagens para decidir se manda um ACK ou NACK para o servidor.
* Em caso de mensagens corrompidas, a função parseChannelPackage retorna 0, o que também causa o envio de um NACK
* para o servidor.
*/
void listenServer () {
	while (1) {
		char serverPackage[CHANNEL_PACKAGE_SIZE]; // Buffer para pacotes recebidos do servidor.

		receiveData(serverPackage);	// Aguarda por mensagens do servidor.

		struct channelMessage serverMessage; // Estrutura de dados que armazena os dados das mensagens do servidor.

		int status = parseChannelPackage(serverPackage, &serverMessage); // Coloca os dados recebidos na estrutura de dados.
		
		if (status == 1) {
			if (serverMessage.seqNumber != lastReceivedSeqNumber) {
				insertRecord(serverMessage.message, (struct timeval *) &serverMessage.timestamp);
				lastReceivedSeqNumber = serverMessage.seqNumber;
			}
			sendMessage(serverMessage.seqNumber, ACK_MSG, DUMMY_MESSAGE);
		} else {
			sendMessage(serverMessage.seqNumber, NACK_MSG, DUMMY_MESSAGE);
		}
	}
}
