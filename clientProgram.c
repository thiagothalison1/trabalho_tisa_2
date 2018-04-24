#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timestamp.h"
#include "channelClient.h"
#include "circularList.h"
#include "bufferControl.h"

/*** Constants ***/

#define NSEC_PER_SEC (1000000000) // Representa a quantidade de nanosegundos por segundo.

/*** Global Variables ***/

pthread_t dataHandler, clientChannel, logGenerator;

/*** Functions ***/

/**
 * Função responsável pelo armazenamento dos dados recebidos em um arquivo para posterior consulta.
 */
void * logInfo() {
    FILE *file;

    while (1) {
        // Espera um dos buffers do bufferControl estiver cheio de dados para então escrevê-los.
        // Esta espera ocorre para evitar que operações de escrita sejam realizadas deliberadamente comprometendo
        // assim o processamento.
        struct buffer_data * fullBuffer = waitFullBuffer(); 

        file = fopen("./serverLog.txt", "a");

        struct buffer_data * data;

        for (int i = 0; i < TAMBUF; i++) {
            data = &fullBuffer[i];

            // Recupera o timestamp em um formato de data amigável.
            char formattedTime[FORMATTED_TIME_SIZE];
            getFormattedTime((struct timeval *) &data->timestamp, (char *) &formattedTime);

            fprintf(file, "Value sent: %c  -  Time: %s\n", data->data, formattedTime);
        }

        fclose(file);
    }
}

/**
 * Esta função é responsável pela manipualção dos dados recebidos pelo canal de comunicação.
 * Ela fica constantemente lendo os dados disponíveis para printá-los na tela.
 * Além disso ela armazena eles no buffer de log para que sejam gravados periodicamente.
 */
void * handleData(void * arg) {
    while (1) {
        struct record recordValue;

        // Lê o próximo dado disponível.
        readRecord(&recordValue);

        // Armazena os valores lidos no buffer utilizado pelo log.
        insertValue(recordValue.data, (struct timeval *) &recordValue.timestamp);

        // Recupera o timestamp em uma versão mais amigável para apresentação.
        char formattedTime[FORMATTED_TIME_SIZE];
        getFormattedTime((struct timeval *) &recordValue.timestamp, (char *) &formattedTime);

        // Printa o dado recebido na tela.
		printf("Value received: %c  -  Time: %s\n", recordValue.data, formattedTime);
	}
}


/**
 * Esta função é pelo inicio da comunicação.
 */
void * startClientChannel(void * arg) {
	startClient();
	listenServer();
}

int main(int argc, char *argv[]) {
    // Thread para funcção de tratamento de dados.
    pthread_create(&dataHandler, NULL, (void *) handleData, NULL);

    // Thread para canal de comunicação com o cliente.
	pthread_create(&clientChannel, NULL, (void *) startClientChannel, NULL);

    // Thread para gravação de dados recebidos.
    pthread_create(&logGenerator, NULL, (void *) logInfo, NULL);

    pthread_join(dataHandler, NULL);
	pthread_join(clientChannel, NULL);
    pthread_join(logGenerator, NULL);
}
