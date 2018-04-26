/************************************************************************************************
* Esta interface é utilizada em serverProgram.c para a comunicação com o arduino via
* porta serial.
*************************************************************************************************/
int serialPortInit(const char* serialport);

int serialPortReadUntil(int fd, char* buf, char until);