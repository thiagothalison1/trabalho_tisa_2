#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timestamp.h"
#include "channelServer.h"
#include "serialReader.h"
#include "circularList.h"
#include "bufferControl.h"

/* Constants */

#define NSEC_PER_SEC (1000000000) // Número de nanosegundos or segundo.

/* Global Variables */

pthread_t serialReadController, messageSenderController, logGenerator;


/* Functions */

/**
 * Função responsável por fazer uma thread "dormir" por um intervalo definido de temp.
 * Parâmetros: int milisecInterval: Tempo de sleep milisegundos.
 *             struct timespec *t: Estrutura que representa o "cronômetro" para despertar a thread.
 */
void alarmClock(int milisecInterval, struct timespec *t) {
    t->tv_nsec += milisecInterval * 1000000;

    while (t->tv_nsec >= NSEC_PER_SEC) {
        t->tv_nsec -= NSEC_PER_SEC;
        t->tv_sec++;
    }

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
}

/**
 * Função responsável pela gravação de dados em um arquivo de dados.
 * A função faz a gravação dos dados de 10 em 10, para isto utilizada um mecanismo de buffer duplo.
 */
void * logInfo() {
    FILE *file;

    while (1) {
        struct buffer_data * fullBuffer = waitFullBuffer();

        file = fopen("./clientLog.txt", "a");

        struct buffer_data * data;

        for (int i = 0; i < 10; i++) {
            // Recupera o próximo dado no buffer duplo.
            data = &fullBuffer[i];

            // Gera uma representação amigável do timestamp do dado
            char formattedTime[FORMATTED_TIME_SIZE];
            getFormattedTime((struct timeval *) &data->timestamp, (char *) &formattedTime);

            // Grava as informações em um arquivo.
            fprintf(file, "Value sent: %c  -  Time: %s\n", data->data, formattedTime);
        }

        fclose(file);
    }
}

/**
 * Função responsável pela leitura de dados oriundos do arduino. Os dados lidos são inseridos em 
 * duas listas: uma lista circular utilizada para comunicação com o serverChannel, e um buffer
 * duplo utilizado para comunicação com a thread de gravação de dados.
 */
void * serialRead(void * arg) {
    int readerPeriod = 1000;
    struct timespec readerClock;
    clock_gettime(CLOCK_MONOTONIC ,&readerClock);
    
    int fd = 0;
    char serialport[256] = {"/dev/ttyACM0"};
    int baudrate = 9600;  // default
    char buf[256];

    while(1){
        //fd = serialPortInit(serialport);

        // Lê o próximo valor enviado pelo arduino.
        // serialPortReadUntil(fd, buf, '\n');
        // printf("read: %s\n", buf);

        // Recupera o timestamp atual.
        struct timeval timestamp;
        getTime(&timestamp);
        
        // Grava o valor obtido + timestamp na lista circular.
        buf[0] = 'i';
        insertRecord(buf[0], &timestamp);
        // Grava o valor obtido + timestamp no buffer duplo.
        insertValue(buf[0], &timestamp);
        
        // Dorme por um segundo.
        alarmClock(readerPeriod, &readerClock);
    }
}

/**
 * Função responsável pelo envio de dados para o cliente através do canal de comunicação serverChannel.
 * Ela fica lendo os dados na lista circular e mandando para o canal de comunicação.
 */
void * messageSender(void * arg) {
    while (1) {
        struct record recordValue;
        // Lê o próximo dado na lista circular.
        readRecord(&recordValue);

        // Printa na tela o dado + timestamp a serem mandados
        char formattedTime[FORMATTED_TIME_SIZE];
        getFormattedTime((struct timeval *) &recordValue.timestamp, (char *) &formattedTime);
        printf("Value sent: %c  -  Time: %s\n", recordValue.data, formattedTime);

        // Envia a mensagem.
        int result = sendMessage(recordValue.data, (struct timeval *) &recordValue.timestamp);

        // Caso a mensagem não seja enviada, então o servidor para e o usuário é avisado que o cliente está desconectado.
        if (result == 0) {
            puts("Message send failure. Client is probably disconnected");
            exit(0);
        } else {
            // puts("Send success."); Não apagar.
        }
    }
}

int main(int argc, char *argv[]) {
    // Inicia o servidor e espera por uma conexão de cliente.
    startServer();

    puts("Connection Stablished!");

    // Inicia as threads para consumo de dados do arduino, envio para o cliente e geração de log de dados.
    pthread_create(&serialReadController, NULL, (void *) serialRead, NULL);
    pthread_create(&messageSenderController, NULL, (void *) messageSender, NULL);
    pthread_create(&logGenerator, NULL, (void *) logInfo, NULL);

    pthread_join(serialReadController, NULL);
    pthread_join(messageSenderController, NULL);
    pthread_join(logGenerator, NULL);
}