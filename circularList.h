struct record {
 char data;
 struct timeval * timestamp;
};

void insertRecord(char data, struct timeval * timestamp);

struct record * readRecord(struct record * recordValue);