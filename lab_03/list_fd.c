#include "list_fd.h"

void push(node_t **head, int fd)
{
    node_t *list_node = NULL;
    
    list_node = malloc(sizeof(struct node));
    list_node -> fd = fd;
    list_node -> next = *head;
    *head = list_node;
}

node_t* pop(node_t **head)
{
    node_t *list_node = NULL;
    
    list_node = *head;
    *head = (*head) -> next;
    return list_node;
}

void free_list(node_t **head)
{
    node_t *cur = *head;
    node_t *next = NULL;
    
    while (cur != NULL)
    {
        next = cur->next;
        free(cur);
        cur = next;
    }
}
