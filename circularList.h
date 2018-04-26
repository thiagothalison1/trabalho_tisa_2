/************************************************************************************************
* Esta interface é utilizada em serverProgram.c e clienteProgram.c para realizar a comunicação
* entre os canais de comnicação e as camadas de aplicação.
*************************************************************************************************/
struct record {
 char data;
 struct timeval * timestamp;
};

void insertRecord(char data, struct timeval * timestamp);

struct record * readRecord(struct record * recordValue);