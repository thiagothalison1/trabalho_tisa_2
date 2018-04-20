#include "circularList.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NSEC_PER_SEC (1000000000) /* The number of nsecs per sec. */
pthread_t producer, consumer;

void alarmClock(int milisecInterval, struct timespec *t) {
    t->tv_nsec += milisecInterval * 1000000;

    while (t->tv_nsec >= NSEC_PER_SEC) {
        t->tv_nsec -= NSEC_PER_SEC;
        t->tv_sec++;
    }

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
}

void * produce(void * arg) {
    int producePeriod = 500;
    struct timespec produceCLock;
    clock_gettime(CLOCK_MONOTONIC ,&produceCLock);

    while (1) {
        int r = rand() % 100;
        if (r >= 40) {
            insert('a');
        }
        alarmClock(producePeriod, &produceCLock);
    }
}

void * consume(void * arg) {
    int consumePeriod = 500;
    struct timespec consumeClock;
    clock_gettime(CLOCK_MONOTONIC ,&consumeClock);

    while (1) {
        int r = rand() % 100;
        if (r >= 80) {
            readFromList();
            printf("Removed");
        }
        alarmClock(consumePeriod, &consumeClock);
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    pthread_create(&producer, NULL, (void *) produce, NULL);
    pthread_create(&consumer, NULL, (void *) consume, NULL);
    
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
}