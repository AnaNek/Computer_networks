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

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define BUFFER_SIZE 128000

void *client_handler(void *);
char *get_current_date(char *, int);
char date[128];

struct node
{
    int fd;
    struct node *next;
};

struct file_node
{
    char buffer[BUFFER_SIZE];
    struct file_node *next;
};
