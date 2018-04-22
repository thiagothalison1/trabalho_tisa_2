#include <stdio.h>
#include <pthread.h>
#include "timestamp.h"
#include "channelServer.h"
#include "serialReader.h"
#include "circularList.h"
#include "bufferControl.h"

#define NSEC_PER_SEC (1000000000) /* The number of nsecs per sec. */

pthread_t serialReadController, messageSenderController, logGenerator;

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

        file = fopen("./clientLog.txt", "a");

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

void * serialRead(void * arg) {
    int readerPeriod = 1000;
    struct timespec readerClock;
    clock_gettime(CLOCK_MONOTONIC ,&readerClock);
    
    int fd = 0;
    char serialport[256] = {"/dev/ttyACM0"};
    int baudrate = 9600;  // default
    char buf[256];

    while(1){
        fd = serialport_init(serialport, baudrate);
        serialport_read_until(fd, buf, '\n');
        //printf("read: %s\n", buf);

        struct timeval timestamp;
        getTime(&timestamp);

        insertRecord(buf[0], &timestamp);
        insertValue(buf[0], &timestamp);

        alarmClock(readerPeriod, &readerClock);
    }
}

void * messageSender(void * arg) {
    while (1) {
        struct record recordValue;
        readRecord(&recordValue);

        char formattedTime[FORMATTED_TIME_SIZE];
        getFormattedTime((struct timeval *) &recordValue.timestamp, (char *) &formattedTime);

        printf("Value sent: %c  -  Time: %s\n", recordValue.data, formattedTime);

        int result = sendMessage(recordValue.data, (struct timeval *) &recordValue.timestamp);

        if (result == 0) {
            puts("Message send failure.");
        } else {
            // puts("Send success."); Não apagar.
        }
    }
}

int main(int argc, char *argv[])
{
    startServer();

    puts("Connection Stablished!");

    pthread_create(&serialReadController, NULL, (void *) serialRead, NULL);
    pthread_create(&messageSenderController, NULL, (void *) messageSender, NULL);
    pthread_create(&logGenerator, NULL, (void *) logInfo, NULL);

    pthread_join(serialReadController, NULL);
    pthread_join(messageSenderController, NULL);
    pthread_join(logGenerator, NULL);

    //int result = sendMessage('o');

    //if (result == 0) {
    //    puts("Message send failure.");
    //} else {
    //    puts("Send success.");
    //}
}