#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "list_fd.h"

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define BUFFER_SIZE 128000

void *client_handler(void *);
char *get_current_date(char *, int);
void update_stat(void);

char date[128];
char stats[1024];
int server_socket_fd = 0;
node_t * head = NULL;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

char root_dir[128];

