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
};

unsigned short calculateCheckSum(const unsigned char* data_p);

void buildChannelPackage(char messageType, char seqNumber, char message, char * package);

int parseChannelPackage(char * package, struct channelMessage *messageInfo);