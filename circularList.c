#include "circularList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "timestamp.h"

/* Constants */

#define TAMLIST 10

/* Global Variables */

struct record records[TAMLIST]; // Lista.
int emptyList = 1; // Variável que indica quando a lista está vazia.
int fullList = 0; // Variável que indica quando a lista está cheia.
int head = 0; // Variável que aponta para o valor mais recente da lista.
int tail = 0; // Variável que aponta para o valor mais antigo da lista.

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t listNotFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t listNotEmpty = PTHREAD_COND_INITIALIZER;

/**
 * Função responsável pela inserção de dados na lista.
 * Parâmetros: char * data: dado a ser inserido.
 *             struct timeval * timestamp: Timestamp referente a coleta do dado.
 */
void insertRecord(char data, struct timeval * timestamp) {
    pthread_mutex_lock(&mutex);
        // Caso a lista esteja cheia, então a thread espera para inserção de novo dado.
        while (fullList == 1) {
            pthread_cond_wait(&listNotFull, &mutex);
        }

        records[head].data = data;
        memcpy(&records[head].timestamp, timestamp, sizeof(struct timeval));

        head = ++head % TAMLIST;
        if (head == tail) fullList = 1;

        // Se a lista estava anteriormente vazia, então ela é desmarcada como vazia.
        // Qualquer thread que esteja esperando para consumir dados da lista é sinalizada.
        if (emptyList == 1) {
            emptyList = 0;
            pthread_cond_signal(&listNotEmpty);
        }
    pthread_mutex_unlock(&mutex);
}

/**
 * Função responsável pela leitura do dado mais antigo inserido na lista.
 * Parâmetros: struct record * recordValue: estrutura de dados utilizada para gravar as informações
 * do dado e seu respectivo timestamp.
 */
struct record * readRecord(struct record * recordValue) {
    pthread_mutex_lock(&mutex);
        // Caso a lista esteja vazia, então a thread espera até que se tenha dados para consumo.
        while (emptyList == 1) {
            pthread_cond_wait(&listNotEmpty, &mutex);
        }

        recordValue->data = records[tail].data;
        memcpy(&recordValue->timestamp, &records[tail].timestamp, sizeof(struct timeval));

        tail = ++tail % TAMLIST;
        if (tail == head) emptyList = 1;

        // Se a lista estava anteriormente cheia, então ela é desmarcada como cheia.
        // Qualquer thread que esteja esperando para inserir dados na lista é sinalizada.
        if (fullList == 1) {
            fullList = 0;
            pthread_cond_signal(&listNotFull);
        }
    pthread_mutex_unlock(&mutex);
}
