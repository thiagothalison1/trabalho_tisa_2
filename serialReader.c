#include <stdio.h>    
#include <stdlib.h> 
#include <stdint.h>   
#include <string.h>   
#include <unistd.h>   
#include <fcntl.h>    
#include <errno.h>    
#include <termios.h>  
#include <sys/ioctl.h>
#include <getopt.h>
#include <time.h>
#include "serialReader.h"

/* Constants */

#define NSEC_PER_SEC (1000000000) //Número de nanosegundos por segundo.

/* Functions */

/**
 * Função responsável pela escrita de bytes na porta serial.
 * Parâmetros: int fd: Descritor de arquivo de comunicação com a porta serial.
 *             uint8_t b: byte a ser escrito..
 */ 
int serialPortWriteByte(int fd, uint8_t b){
    int n = write(fd,&b,1);
    if( n!=1)
        return -1;
    return 0;
}

/**
 * Função responsável pela escrita de textos na porta serial.
 * Parâmetros: int fd: Descritor de arquivo de comunicação com a porta serial.
 *             const char* str: Texto a ser escrito na porta serial.
 */ 
int serialPortWrite(int fd, const char* str) {
    int len = strlen(str);
    int n = write(fd, str, len);
    if( n!=len ) 
        return -1;
    return 0;
}

/**
 * Lê bytes recebidos na porta serial até um certo número de bytes.
 * Parâmetros: int fd: Descritor de arquivo de comunicação com a porta serial.
 *             char* buf: Buffer no qual os dados recebidos na serial são gravados.
 *             char until: Número de bytes a serem lidos.
 */ 
int serialPortReadUntil(int fd, char* buf, char until) {
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

/**
 * Inicia a porta serial.
 * Parâmetros: const char* serialport: identificador da porta serial.
 */    
int serialPortInit(const char* serialport) {
    int fd;
       
    fd = open(serialport, O_RDONLY);
    if (fd == -1)  {
        perror("Não foi possível abrir a porta serial");
        return -1;
    } 
}
