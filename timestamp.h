#include <time.h>
#include <sys/time.h>

#define FORMATTED_TIME_SIZE 30

void getTime (struct timeval * timeNow);

void getFormattedTime (struct timeval * time, char * formattedTime);