#ifndef LIST_FD_H
#define LIST_FD_H

#include <stdlib.h>

typedef struct node
{
    int fd;
    struct node *next;
} node_t;

void push(node_t **head, int fd);
node_t* pop(node_t **head);
void free_list(node_t **head);

#endif
