#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timestamp.h"
#include "channelClient.h"
#include "circularList.h"
#include "bufferControl.h"

#define NSEC_PER_SEC (1000000000) /* The number of nsecs per sec. */
pthread_t userInterface, clientChannel, logGenerator;

void alarmClock(int milisecInterval, struct timespec *t) {
    t->tv_nsec += milisecInterval * 1000000;

    while (t->tv_nsec >= NSEC_PER_SEC) {
        t->tv_nsec -= NSEC_PER_SEC;
        t->tv_sec++;
    }

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
}

void * logInfo() {
    FILE *file;

    while (1) {
        struct buffer_data * fullBuffer = waitFullBuffer();

        file = fopen("./serverLog.txt", "a");

        struct buffer_data * data;

        for (int i = 0; i < 10; i++) {
            data = &fullBuffer[i];
            // printf("Value sent: %c  -  Time: %s\n", recordValue.data, formattedTime);
            char formattedTime[FORMATTED_TIME_SIZE];
            getFormattedTime((struct timeval *) &data->timestamp, (char *) &formattedTime);

            fprintf(file, "Value sent: %c  -  Time: %s\n", data->data, formattedTime);
        }

        fclose(file);
    }
}

/**
 * Esta função é responsável por apresentar dados na tela.
 */
void * startUX(void * arg) {
    while (1) {
        struct record recordValue;
        readRecord(&recordValue);

        insertValue(recordValue.data, (struct timeval *) &recordValue.timestamp);

        char formattedTime[FORMATTED_TIME_SIZE];
        getFormattedTime((struct timeval *) &recordValue.timestamp, (char *) &formattedTime);
		printf("Value received: %c  -  Time: %s\n", recordValue.data, formattedTime);
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
    pthread_create(&logGenerator, NULL, (void *) logInfo, NULL);

    pthread_join(userInterface, NULL);
	pthread_join(clientChannel, NULL);
    pthread_join(logGenerator, NULL);
}
