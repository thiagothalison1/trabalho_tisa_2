#include "bufferControl.h"
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

/*** Global Variables ***/

struct buffer_data buffer[2][TAMBUF]; 

int bufferFree = 0; // Índice do buffer disponível para inserção de dados.
int bufferFreeIndex = 0; // Índice para inserção de dado disponível no buffer sendo "alimentado".
int bufferFull = -1; // índice do buffer disponível para "consumo de dados.

pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bufferFullAlarm = PTHREAD_COND_INITIALIZER;

/*** Functions ***/

/**
* Esta função é responsável pela inserção de valores em um buffer que é utilizado para registro de valores em arquivo. 
* Parâmetros: char data: dado
*             struct timeval * timestamp: dados de tempo da mensagem
*/
void insertValue(char data, struct timeval * timestamp) {
    pthread_mutex_lock(&bufferMutex);
        
        // Atualiza o buffer com os dados recebidos na próxima posição livre.
        buffer[bufferFree][bufferFreeIndex].data = data;
        memcpy(&buffer[bufferFree][bufferFreeIndex].timestamp, timestamp, sizeof(struct timeval));

        bufferFreeIndex++;

        // Caso o buffer fique cheio, então a thread que está aguardando por buffer cheio é liberada para
        // comsumo dos dados.
        if (bufferFreeIndex == TAMBUF) {
            bufferFull = bufferFree;
            bufferFree = (bufferFree + 1) % 2;
            bufferFreeIndex = 0;
            pthread_cond_signal(&bufferFullAlarm);
        }
    pthread_mutex_unlock(&bufferMutex);
}

/**
* Esta função faz com que threads consumidores de dados aguardem até que um dos buffers do buffer duplo fique.
* Quando um buffer fica cheio, a thread que está esperando os dados é liberada para consumí-los. 
* pthread_cond_wait (bufferFullAlarm) e liberar a thread bufferMutex.
*/
struct buffer_data * waitFullBuffer (void) {
    struct buffer_data * bufferFullPointer;

    pthread_mutex_lock(&bufferMutex);
        while (bufferFull == -1){
            pthread_cond_wait(&bufferFullAlarm, &bufferMutex);
        }

        // Recupera o ponteiro para o buffer que estiver completo (todas as posições com dados)
        // e armazena em uma variável de retorno da função.
        bufferFullPointer = (struct buffer_data *) &buffer[bufferFull];
        bufferFull = -1;
    pthread_mutex_unlock(&bufferMutex);

    return bufferFullPointer;
}


