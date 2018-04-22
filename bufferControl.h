struct buffer_data
{
    char data;
    struct timeval * timestamp;
};

void insertValue(char data, struct timeval * timestamp);

struct buffer_data * waitFullBuffer (void);