#include <pthread.h>
#include <sys/time.h>

void worker_routine();

static int BUFFER = 1024;

typedef struct {
    pthread_mutex_t lock;
    int account_id;
} account;

typedef struct {
    char command[1024];
    struct timeval time_start;
    int request_id;
} request;

typedef struct {
	request req;
	struct node *next_node;
} node;