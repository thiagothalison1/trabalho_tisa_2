/*
    Simple udp server
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include <unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/time.h>
#include "channelMessageProtocol.h"

#define MAX_SEND_ATTEMPTS 5
#define PORT 8888   //The port on which to listen for incoming data

/**
 * Global Variables
 */
struct sockaddr_in si_me, si_other;
int s, i, slen = sizeof(si_other) , recv_len;
char seqNumber = 0;

struct channelMessage * messageInfo;
 
void die(char *s) {
    perror(s);
    exit(1);
}

void disconnect () {
    close(s);
}

char generateSeqNumber () {
    return ++seqNumber % 255;
}

void createServer () {
    /* Create an UDP socket */
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    /* zero out the structure */
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    /* bind socket to port */
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        die("Set socket timeout");
    }
}

int receiveData (struct channelMessage * messageInfo) {
    char buf[CHANNEL_PACKAGE_SIZE];
    /* Try to receive some data, this is a blocking call */
    if ((recv_len = recvfrom(s, buf, CHANNEL_PACKAGE_SIZE + 1, 0, (struct sockaddr *) &si_other, &slen)) < 0) {
        /* When timeout occurs */
        return 0;
    }
        
    parseChannelPackage(buf, messageInfo);

    /* When the message was successfully received */
    return 1;
}

void createDataPackage (char message, struct timeval * timeNow, char * package) {
    char seqNumber = generateSeqNumber();

    buildChannelPackage(DATA_MSG, seqNumber, message, timeNow, package);
}

int dispatch (char * package) {
    /* Send data to connected client */
    if (sendto(s, package, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1) {
        die("sendto()");
    }

    struct channelMessage response;
    
    int receiveStatus = receiveData(&response);

    if (receiveStatus == 1) {
        if (response.msgType == ACK_MSG) {
            return 1;
        }
        return 0;
    }

    return 0;
}

int sendMessage (char message, struct timeval * timeNow) {
    char package[CHANNEL_PACKAGE_SIZE];

    createDataPackage(message, timeNow, package);

    int messageSuccess = 0;

    int sendAttempts = 0;

    while (messageSuccess == 0 && sendAttempts != MAX_SEND_ATTEMPTS) {
        messageSuccess = dispatch(package);
        sendAttempts++;
    }
    
    if (sendAttempts == MAX_SEND_ATTEMPTS) {
        return 0;
    } else {
        return 1;
    }
}

void waitConnection () {
    int hasConnection = 0;

    while (!hasConnection) {
        fflush(stdout);
        
        struct channelMessage messageInfo;

        int result = receiveData(&messageInfo);

        if (result == 1) {
            if (messageInfo.msgType == CONNECT_MSG) {
                hasConnection = 1;
            }
        }
    }
}

void startServer () {
    createServer();
    waitConnection();
}
 