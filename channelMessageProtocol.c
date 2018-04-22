#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "channelMessageProtocol.h"

/**
 * Constants
 */
char CONNECT_MSG = '1';
char DISCONNECT_MSG = '0';
char ACK_MSG = 'a';
char NACK_MSG = 'n';
char DATA_MSG = 'd';
int CHANNEL_PACKAGE_SIZE = 5 + sizeof(struct timeval);

char DUMMY_SEQ_NUMBER = '0';
char DUMMY_MESSAGE = '#';

int MESSAGE_TYPE_INDEX = 0;
int SEQ_NUMBER_INDEX = 1;
int CHECKSUM_INDEX = 2;
int MESSAGE_INDEX = 4;
int TIMESTAMP_INDEX = 5;

/**
 * Functions
 */
unsigned short calculateCheckSum(const unsigned char* data_p) {
    unsigned char length = CHANNEL_PACKAGE_SIZE;
    unsigned char x;
    unsigned short checkSum = 0xFFFF;

    while (length--){
        x = checkSum >> 8 ^ *data_p++;
        x ^= x >> 4;
        checkSum = (checkSum << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return checkSum;
}

void buildChannelPackage(char messageType, char seqNumber, char message, struct timeval * timestamp, char * package) {
    package[MESSAGE_TYPE_INDEX] = messageType;

    package[SEQ_NUMBER_INDEX] = seqNumber;

    /* Fill out checksum position with zeroes */
    memset(&package[CHECKSUM_INDEX], 0, 2);

    package[MESSAGE_INDEX] = message;

    memcpy(&package[TIMESTAMP_INDEX], timestamp, sizeof(struct timeval));
    
    /* Calculate checksum for entire package and place it at checksum position */
    unsigned short checkSum = calculateCheckSum(package);
    memcpy(&package[CHECKSUM_INDEX], &checkSum, 2);
}

int parseChannelPackage(char * package, struct channelMessage *messageInfo) {
    /* Store expected checksum */
    unsigned short expectedChecksum;
    memcpy((unsigned short *) &expectedChecksum, &package[CHECKSUM_INDEX], 2);

    /* Fill out checksum position with zeroes for checksum calculation */
    memset(&package[CHECKSUM_INDEX], 0, 2);

    unsigned short currentChecksum = calculateCheckSum(package);

    if (expectedChecksum != currentChecksum) {
        return 0;
    } else {
        messageInfo->msgType = package[MESSAGE_TYPE_INDEX];
        messageInfo->seqNumber = package[SEQ_NUMBER_INDEX];
        messageInfo->checkSum = expectedChecksum;
        messageInfo->message = package[MESSAGE_INDEX];
        memcpy(&messageInfo->timestamp, &package[TIMESTAMP_INDEX], sizeof(struct timeval));
        return 1;
    }
}