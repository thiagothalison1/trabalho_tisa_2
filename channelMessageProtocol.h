/************************************************************************************************
* Esta interface implementa o protocolo de comunicação entre cliente e servidor. A função 
* buildChannelPackage é utilizada para montagem de mensagens e a função parseChannelPackage para
* interpretação de mensagens. É neste nível do programa que o checksum é verificado para detecção
* de pacotes de dados corrompidos.
*************************************************************************************************/
extern char CONNECT_MSG;
extern char DISCONNECT_MSG;
extern char ACK_MSG;
extern char NACK_MSG;
extern char DATA_MSG;
extern int CHANNEL_PACKAGE_SIZE;
extern char DUMMY_SEQ_NUMBER;
extern char DUMMY_MESSAGE;

struct channelMessage {
    char msgType;
    char seqNumber;
    unsigned short checkSum;
    char message;
    struct timeval * timestamp;
};

unsigned short calculateCheckSum(const unsigned char* data_p);

void buildChannelPackage(char messageType, char seqNumber, char message, struct timeval * timestamp, char * package);

int parseChannelPackage(char * package, struct channelMessage *messageInfo);