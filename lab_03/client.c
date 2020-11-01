#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 128000

// ./client address port

int main(int argc, char *argv[])
{

    char *address = argv[1];
    int port = atoi(argv[2]);
    int rc = 0;
    int client_socket_fd;


    //Создание сокета

    if ((client_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in rem_address;
    struct hostent * rem;

    rem_address.sin_family = AF_INET;
    rem_address.sin_port = htons(port);

    if ((rem = gethostbyname(address)) == NULL)
    {
        herror("Error in gethostbyname");
        exit(EXIT_FAILURE);
    }

    memcpy (&rem_address.sin_addr, rem -> h_addr, rem -> h_length);

    if (connect(client_socket_fd, (struct sockaddr *) &rem_address, sizeof(rem_address)) < 0)
    {
        perror("Error connecting");
        exit(EXIT_FAILURE);
    }

    char request[1024];

    puts("Give a server request...");
    fgets(request, sizeof(request), stdin);

    char response[BUFFER_SIZE];

    send(client_socket_fd, request, sizeof(request), 0);

    rc = recv(client_socket_fd, &response, sizeof(response), MSG_WAITALL);
    if (!rc)
    {
        printf("Connection reset\n");
    }

    printf("\n%s\n", response);

    close(client_socket_fd);

    return 0;
}
