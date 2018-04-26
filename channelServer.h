/************************************************************************************************
* Esta interface é utilizada em serverProgram.c para realizar a comunicação
* com o cliente.
*************************************************************************************************/
void startServer();

int sendMessage (char message, struct timeval * timeNow);