#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/time.h>
#include "channelMessageProtocol.h"

/* Constants */

#define MAX_SEND_ATTEMPTS 5 // Número máximo de tentativas de envio de pacote repetido.
#define PORT 8888 // A porta que o servidor utiliza para escutar dados.
#define RESPONSE_TIMEOUT 3 // Timeout de espera por respostas do cliente.

/* Global Variables */

struct sockaddr_in serverSocket, clientSocket;
int serverSocketFd, clientSocketLen = sizeof(clientSocket), receivedMessageLength;
char seqNumber = 0;
struct channelMessage * messageInfo;

/* functions */

/**
 * Função responsável por matar o processo em caso de erros na criação do socket do servidor.
 * Parâmetros: char * s: String de mensagem de erro.
 */
void die(char *s) {
    perror(s);
    exit(1);
}

/**
 * Função responsável por fechar o socket de conexão do servidor.
 */
void disconnect () {
    close(serverSocketFd);
}

/**
 * Função responsável pela geração do número de sequência do próximo pacote a ser enviado.
 * O campo para número de sequência é um char (8 bits), sendo assim, os números de sequência
 * variam entre 0 e 255. 
 */
char generateSeqNumber () {
    return ++seqNumber % 256;
}

/**
 * Função responsável por atribuir um timeout para recebimento de mensagens no socket do servidor.
 * O clock do timeout é acionado toda vez que a função recvfrom é chamada. ver: receiveData.
 */
void attachSocketTimeout () {
    struct timeval tv;
    tv.tv_sec = RESPONSE_TIMEOUT;
    tv.tv_usec = 0;

    if (setsockopt(serverSocketFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        die("Set socket timeout");
    }
}

/**
 * Função responsável por criar o socket do servidor.
 */
void createServer () {
    // Cria um socket utilizando UDP.
    if ((serverSocketFd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    // Preenche a estrutura de dados do socket com zeros para evitar "lixos".
    memset((char *) &serverSocket, 0, sizeof(serverSocket));
     
    serverSocket.sin_family = AF_INET; // Família de endereço.
    serverSocket.sin_port = htons(PORT); // Porta do endereço.
    serverSocket.sin_addr.s_addr = htonl(INADDR_ANY); // Endereço.
     
    // Faz a ligação do socket com a porta do processo do servidor.
    if (bind(serverSocketFd , (struct sockaddr*)&serverSocket, sizeof(serverSocket) ) == -1) {
        die("bind");
    }

    // Chama a função responsável por atribuir timeout no socket.
    attachSocketTimeout();
}

/**
 * Função responsável pelo recebimento de mensagens do cliente.
 * Parâmetros: struct channelMessage * messageInfo: estrutura para armazenar dados da mensagem recebida.
 */
int receiveData (struct channelMessage * messageInfo) {
    char buf[CHANNEL_PACKAGE_SIZE];
    
    // Fica esperando mensagens de retorno do cliente.
    if ((receivedMessageLength = recvfrom(serverSocketFd, buf, CHANNEL_PACKAGE_SIZE + 1, 0, (struct sockaddr *) &clientSocket, &clientSocketLen)) < 0) {
        // Se ocorrer timeout (espera maior que 3 segundos), então a função retorna false. 
        return 0;
    }
        
    // Caso alguma mensagem seja recebida, o parser da mensagem é executado e o retorno do parser será o retorno da função.
    // O parser retorna true se o checksum está ok e falso se existe alguma falha de checksum.
    int parserResult =  parseChannelPackage(buf, messageInfo);

    return parserResult;

}

/**
 * Função responsável pela criação de pacotes para envio. Os pacotes de envio devem seguir o protocolo 
 * defido em channelMessageProtocol.c.
 * Parâmetros: char message: Mensagem a ser enviada.
 *             struct timeval * timeNow: timestamp referente a coleta da mensagem.
 *             char * package: buffer onde serão gravados os bytes referentes a mensagem enviada.
 */
void createDataPackage (char message, struct timeval * timeNow, char * package) {
    char seqNumber = generateSeqNumber();

    //FALHA 3 - PERGUNTA 5, 6, 7
    //seqNumber = '2';

    buildChannelPackage(DATA_MSG, seqNumber, message, timeNow, package);
}

/**
 * Função responsável pelo envio de mensagens para o cliente.
 * Para cada pacote de dado enviado é esperado um ACK com o mesmo número de pacote (protocolo stop and wait).
 * Parâmetros: char * package: buffer contendo os dados a serem enviados.
 */
int dispatch (char * package) {
    // Envia dados para o cliente.
    if (sendto(serverSocketFd, package, CHANNEL_PACKAGE_SIZE, 0, (struct sockaddr*) &clientSocket, clientSocketLen) == -1) {
        die("sendto()");
    }

    struct channelMessage response;
    
    // Recebe a resposta do cliente.
    int receiveStatus = receiveData(&response);

    // Faz o tratamento da resposta.
    if (receiveStatus == 1) {
        // A mensagem só é considerada como enviada com sucesso quando a mensagem de resposta é um ACK e o
        // número de sequência é igual ao número de sequência do pacote enviado.
        if (response.msgType == ACK_MSG && response.seqNumber == seqNumber) {
            return 1;
        }
        return 0;
    }

    return 0;
}

/**
 * Função responsável pela criação e envio de pacotes de mensagens para o cliente.
 * São realizadas até 5 tentativas de enviar o mesmo pacote. Caso as 5 tentativas resultem em falha, então esta função 
 * retorna false e o server considerada que o cliente está desconectado.
 * Parâmetros: char message: mensagem a ser enviada.
 *             struct timeval * timeNow: timestamp referente ao tempo de coleta da mensagem.
 */
int sendMessage (char message, struct timeval * timeNow) {
    char package[CHANNEL_PACKAGE_SIZE];

    createDataPackage(message, timeNow, package);

    int messageSuccess = 0;

    int sendAttempts = 0;

    // A função tenta enviar o pacote de dados até um máximo de 5 vezes.
    // Se em 5 tentativas um pacote de ACK (Não Corrompido) não for recebido,
    // então a função retorna false e para de tentar reenviar o pacote. 
    while (messageSuccess == 0 && sendAttempts != MAX_SEND_ATTEMPTS) {
        messageSuccess = dispatch(package);
        sendAttempts++;
        printf("Attempt: %d\n", sendAttempts);
    }
    
    if (sendAttempts == MAX_SEND_ATTEMPTS) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * Função responsável por aguardar conexão do cliente.
 * O servidor só começa a enviar dados para o cliente quando este se conecta com o servidor.
 */
void waitConnection () {
    int hasConnection = 0;

    while (!hasConnection) {
        fflush(stdout); 
        
        struct channelMessage messageInfo;

        int result = receiveData(&messageInfo);

        if (result == 1) {
            if (messageInfo.msgType == CONNECT_MSG) {
                hasConnection = 1;
            }
        }
    }
}

/**
 * Função responsável por criar o servidor. 
 * Primeiramente é realizado o setUp e criação do socket do servidor.
 * Depois o server entra em um loop para espera de conexão do cliente e só então a função retorna.
 * Desta maneira o servidor não ficara gastando processamento com a coleta e envio de dados se não há clientes conectados.
 */
void startServer () {
    createServer();
    waitConnection();
}
 