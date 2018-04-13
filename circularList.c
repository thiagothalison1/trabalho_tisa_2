#include "circularList.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define TAMLIST 10

int data [TAMLIST];

int emptyList = 1;
int fullList = 0;
int head = 0;
int tail = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t listNotFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t listNotEmpty = PTHREAD_COND_INITIALIZER;

int calcListSize() {
    if (head > tail) {
        return head - tail;
    } else {
        return TAMLIST - tail + head;
    }
}

void printList() {
    int listSize = (fullList == 1) ? TAMLIST : (emptyList == 1) ? 0 : calcListSize();

    for (int i=0; i < listSize; i++) {
        int index = (tail + i) % TAMLIST; 
        printf("%d ", data[index]);
    }

    printf("    Length: %d", listSize);
    printf("    tail: %d", tail);
    printf("    head: %d", head);
    printf("\n");
}

void insert(int element) {
    pthread_mutex_lock(&mutex);
        while (fullList == 1) {
            pthread_cond_wait(&listNotFull, &mutex);
        }

        data[head] = element;

        head = ++head % TAMLIST;
        if (head == tail) fullList = 1;

        if (emptyList == 1) {
            emptyList = 0;
            pthread_cond_signal(&listNotEmpty);
        }

        printf("Insertion ");
        printList();
    pthread_mutex_unlock(&mutex);
}

int readFromList() {
    int removedElement;

    pthread_mutex_lock(&mutex);
        while (emptyList == 1) {
            pthread_cond_wait(&listNotEmpty, &mutex);
        }

        removedElement = data[tail];
        tail = ++tail % TAMLIST;
        if (tail == head) emptyList = 1;

        if (fullList == 1) {
            fullList = 0;
            pthread_cond_signal(&listNotFull);
        }
    pthread_mutex_unlock(&mutex);
    
    return removedElement;
}
