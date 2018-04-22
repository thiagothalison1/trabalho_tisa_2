#include <stdio.h>
#include "timestamp.h"
#include "string.h"

void getTime (struct timeval * timeNow) {
    gettimeofday(timeNow, NULL);
}

void getFormattedTime (struct timeval * time, char * formattedTime) {
    struct tm *localTime;
    localTime = localtime(&time->tv_sec);
    strftime(formattedTime, FORMATTED_TIME_SIZE, "%Y:%m:%dT%H:%M:%S", localTime);
    strcat(formattedTime, ".");

    int milisseconds = (int)time->tv_usec / 1000;
    char milissecondsText[6];
    sprintf(milissecondsText, "%dZ", milisseconds);
    strcat(formattedTime, milissecondsText);
    // printf("%s",buf);
} 