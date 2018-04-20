#include<stdio.h>
#include <time.h>
#include <pthread.h>
#include "channelServer.h"
#include "serialReader.h"
#include "circularList.h"

#define NSEC_PER_SEC (1000000000) /* The number of nsecs per sec. */

pthread_t serialReadController, messageSenderController;

void alarmClock(int milisecInterval, struct timespec *t) {
    t->tv_nsec += milisecInterval * 1000000;

    while (t->tv_nsec >= NSEC_PER_SEC) {
        t->tv_nsec -= NSEC_PER_SEC;
        t->tv_sec++;
    }

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
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
        printf("read: %s\n", buf);
        insert(buf[0]);
        alarmClock(readerPeriod, &readerClock);
    }
}

void * messageSender(void * arg) {
    while (1) {
        char message = readFromList();
        int result = sendMessage(message);

        if (result == 0) {
            puts("Message send failure.");
        } else {
            puts("Send success.");
        }
    }
}

int main(int argc, char *argv[])
{
    startServer();

    puts("Connection Stablished!");

    pthread_create(&serialReadController, NULL, (void *) serialRead, NULL);
    pthread_create(&messageSenderController, NULL, (void *) messageSender, NULL);

    pthread_join(serialReadController, NULL);
    pthread_join(messageSenderController, NULL);

    //int result = sendMessage('o');

    //if (result == 0) {
    //    puts("Message send failure.");
    //} else {
    //    puts("Send success.");
    //}
}