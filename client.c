#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "channelClient.h"
#include "circularList.h"

#define NSEC_PER_SEC (1000000000) /* The number of nsecs per sec. */
pthread_t userInterface, clientChannel;

void alarmClock(int milisecInterval, struct timespec *t) {
    t->tv_nsec += milisecInterval * 1000000;

    while (t->tv_nsec >= NSEC_PER_SEC) {
        t->tv_nsec -= NSEC_PER_SEC;
        t->tv_sec++;
    }

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
}

void * startUX(void * arg) {
    while (1) {
		char valueReceived = readFromList();
		printf("Value received: %c\n", valueReceived);
	}
}

void * startClientChannel(void * arg) {
	startClient();
	listenServer();
}

int main(int argc, char *argv[])
{
    pthread_create(&userInterface, NULL, (void *) startUX, NULL);
	pthread_create(&clientChannel, NULL, (void *) startClientChannel, NULL);

    pthread_join(userInterface, NULL);
	pthread_join(clientChannel, NULL);
}
