#include <stdio.h>    /* Standard input/output definitions */
#include <stdlib.h> 
#include <stdint.h>   /* Standard types */
#include <string.h>   /* String function definitions */
#include <unistd.h>   /* UNIX standard function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */
#include <termios.h>  /* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include <getopt.h>
#include <time.h>
#include "serialReader.h"

#define NSEC_PER_SEC (1000000000) /* The number of nsecs per sec. */

int serialport_init(const char* serialport, int baud);
int serialport_writebyte(int fd, uint8_t b);
int serialport_write(int fd, const char* str);
int serialport_read_until(int fd, char* buf, char until);

int serialport_writebyte( int fd, uint8_t b){
    int n = write(fd,&b,1);
    if( n!=1)
        return -1;
    return 0;
}
int serialport_write(int fd, const char* str) {
    int len = strlen(str);
    int n = write(fd, str, len);
    if( n!=len ) 
        return -1;
    return 0;
}

int serialport_read_until(int fd, char* buf, char until) {
    int infoReaderPeriod = 100;
    struct timespec infoReaderClock;
    clock_gettime(CLOCK_MONOTONIC ,&infoReaderClock);

    char b[1];
    int i=0;
    int readStarted = 0;
    int n;

       while( readStarted == 0 || b[0] != until ) {
            n = read(fd, b, 1);  // read a char at a time
            if( n==-1) {
              usleep( 10 * 1000 ); // error, just keep waiting
                continue;
            }
            if( n==0 ) {
               usleep( 10 * 1000 ); // wait 10 msec try again
                continue;

            }
            
            if (b[0] == 'I') {
                readStarted = 1;
                usleep( 10 * 1000 ); 
            }
            
            if (readStarted == 1 && b[0] != 'I') {
               buf[i] = b[0]; 
               i++;
               //usleep( 1 * 1000 ); 
            }
        } 
        buf[i] = 0;  // null terminate the string

       return 0;
    }
    
int serialport_init(const char* serialport, int baud){
    int fd;
       
    fd = open(serialport, O_RDONLY);
    if (fd == -1)  {
        perror("init_serialport: Unable to open port ");
        return -1;
    } 
}

// int main(int argc, char *argv[]) {
//     int readerPeriod = 1000;
//     struct timespec readerClock;
//     clock_gettime(CLOCK_MONOTONIC ,&readerClock);
    
//     int fd = 0;
//     char serialport[256] = {"/dev/ttyACM0"};
//     int baudrate = B9600;  // default
//     char buf[256];

//     while(1){
//         fd = serialport_init(serialport, baudrate);
//         serialport_read_until(fd, buf, '\n');
//         printf("read: %s\n", buf);
//         insert(buf[0]);
//         alarmClock(readerPeriod, &readerClock);
//     }
// }