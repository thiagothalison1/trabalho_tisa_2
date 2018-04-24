/************************************************************************************************
* Esta interface é utilizada em clientProgram.c e em serverProgram.c para realizar a 
* gravação nos arquivos de log: serverLog.txt (serverProgram.c) e clientLog.txt (clientProgram.c)
*************************************************************************************************/
#define TAMBUF 10

struct buffer_data
{
    char data;
    struct timeval * timestamp;
};

void insertValue(char data, struct timeval * timestamp);

struct buffer_data * waitFullBuffer (void);