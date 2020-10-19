#include "server.h"

struct file_node* fhead = NULL;
struct node* head = NULL;
int fl = 0;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

char dir[1024];

/* 
   Добавить обработчик сигнала 
   Написать структуру списка и функции для работы с ней
   Кидать в поток указатель на голову списка ???
   Понять, что делать с директорией (как ее нормально передавать)
   Собирать статистику 
   Проверить везде ли все дескрипторы закрыты и освобождена ли память
*/
int main(int argc, char *argv[])
{

    int opt = 0;
    int serving_port = 0;
    int num_of_threads = 0;
    char root_dir[128];
    int i;
    struct file_node* file;
    char http_header[BUFFER_SIZE];
    int server_socket_fd, client_socket_fd;
    struct sockaddr_in address, caddr;
    socklen_t clientlen;
    char request[1024];
    char path[1024];
    //char dir[1024];
    char method[3];
    struct node *list_node;
    int fd;
    int total_pages = 0, total_bytes = 0;
    fd_set set;
    int maxfd, sockfd;
    //int status;
    
    //clock_t begin = clock();
    
    while ((opt = getopt(argc, argv, "p:t:d:")) != -1)
    {
        switch(opt)
        {
        case 'p':
            serving_port = atoi(optarg);
            break;
        case 't':
            num_of_threads = atoi(optarg);
            break;
        case 'd':
            strcpy(root_dir, optarg);
            break;
        }
    }
    
    pthread_t *thread_ids = malloc(num_of_threads * (sizeof(pthread_t)));

    //Create thread pool
    for ( i = 0 ; i < num_of_threads ; i ++)
    {
        pthread_create(&thread_ids[i], NULL, client_handler, &client_socket_fd);
    }
    
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("Error creating socket");
        return EXIT_FAILURE;
    }
    
    clientlen = sizeof(caddr);
    
    fcntl(server_socket_fd, F_SETFL, O_NONBLOCK);
    
    //Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(serving_port);
    
    FD_ZERO(&set);
    FD_SET(server_socket_fd, &set);
         
    printf("ADDRESS %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    if (bind(server_socket_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
    {
        perror("Error in bind");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_socket_fd, 6) < 0)
    {
        perror("Error in listen");
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        sleep(1);
        puts("Waiting for connections...");
        
        strcpy(dir, root_dir);

        if (select(server_socket_fd + 1, &set, NULL, NULL, NULL) <= 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
                
        clientlen = sizeof(caddr);
        if ((client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &caddr, &clientlen)) < 0)
        {
            perror("Error in accept");
            exit(EXIT_FAILURE);
        }
        getsockname(client_socket_fd, (struct sockaddr *) &caddr, &clientlen);
        printf("Client connected to port: %d\n", ntohs(caddr.sin_port));
        
        pthread_mutex_lock(&mtx);
        //fl = 1;
        list_node = malloc(sizeof(struct node));
        list_node -> fd = client_socket_fd;
        list_node -> next = head;
        head = list_node;
        
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mtx);
        
    }


    return 0;
}


void *client_handler(void *client_fd)
{
    struct node *p;
    struct file_node *f;
    char buffer[BUFFER_SIZE];
    
    int client_socket_fd;
    char root_dir[128];
    int i;
    struct file_node* file;
    char http_header[BUFFER_SIZE];
    //int server_socket_fd, client_socket_fd;
    struct sockaddr_in address, caddr;
    socklen_t clientlen;
    char request[1024];
    char path[1024];
    //char dir[1024];
    char method[3];
    //struct node *p;
    int fd;
    int total_pages = 0, total_bytes = 0;
    //fd_set readfds;
    int maxfd, sockfd;
    
    clock_t begin = clock();
      
    while (1)
    {
		printf("Thread %lu \n", pthread_self());
		pthread_mutex_lock(&mtx);
		    
		while (head == NULL)
		{
		    pthread_cond_wait(&cond, &mtx);
		}
		
		p = head;
		client_socket_fd = p->fd;
        head = head -> next;
        
		//client_socket_fd = *(int *)client_fd;
		printf("Thread %lu started...\n", pthread_self());
		pthread_mutex_unlock(&mtx);
		
		recv(client_socket_fd, &request, sizeof(request), 0);

		sscanf(request,"%s%s", method, path);
		    
		if (strcmp(method, "GET") && strcmp(method, "STATS") && strcmp(method, "SHUTDOWN"))
		{
		    snprintf(http_header, sizeof(http_header), "Usage: GET </siteA/pageA_B.html> OR STATS/SHUTDOWN\n");
		    send(client_socket_fd, http_header, sizeof(http_header), 0);

		    memset(http_header, '\0', sizeof(http_header));
		    memset(path, '\0', sizeof(path));
		    memset(method, '\0', sizeof(method));
		    continue;
		}
		
		if (!strcmp(method, "STATS"))
		{
		    clock_t end = clock();
		    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		    snprintf(http_header, sizeof(http_header), "Server up for %f, served %d pages, %d bytes\n", time_spent, total_pages, total_bytes);
		    send(client_socket_fd, http_header, sizeof(http_header), 0);

		    memset(http_header, '\0', sizeof(http_header));
		    memset(path, '\0', sizeof(path));
		    memset(method, '\0', sizeof(method));
		    continue;
		}
		else if(!strcmp(method, "SHUTDOWN"))
		{
		    printf("Terminating server...\n");
		        //close(server_socket_fd);

		    continue;
		}
		
		strcat(dir, path);
		    
		if ((fd = open(dir, O_RDONLY)) == -1)
		{

		    if (access(dir, R_OK) != 0)
		    {
		        if (errno == EACCES)
		        {
		            snprintf(http_header, sizeof(http_header), "\nHTTP/1.1 403 Forbidden\r\n"
		                         "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
		                         "Date: %s\r\n"
		                         "Content-Type: text/html\r\n"
		                         "Connection: Closed\r\n\r\n",
		                         get_current_date(date, sizeof(date)));
		            send(client_socket_fd, http_header, sizeof(http_header), 0);

		            printf("File \"%s\" is not accessible\n", dir);
		            memset(http_header, '\0', sizeof(http_header));
		            memset(dir, '\0', sizeof(dir));
		            memset(path, '\0', sizeof(path));
		        }
		        else
		        {

		            snprintf(http_header, sizeof(http_header), "\nHTTP/1.1 404 Not Found\r\n"
		                         "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
		                         "Date: %s\r\n"
		                         "Content-Type: text/html\r\n"
		                         "Connection: Closed\r\n\r\n",
		                         get_current_date(date, sizeof(date)));
		            send(client_socket_fd, http_header, sizeof(http_header), 0);

		            printf("File \"%s\" does not exist\n", dir);
		            memset(http_header, '\0', sizeof(http_header));
		            memset(dir, '\0', sizeof(dir));
		            memset(path, '\0', sizeof(path));
		        }
		    }

		    //close(client_socket_fd);
		    pthread_mutex_lock(&mtx);
		    close(client_socket_fd);
		    free(p);
		    pthread_mutex_unlock(&mtx);
		    //return NULL;
		    continue;
		}
		    
		read(fd, buffer, sizeof(buffer));
		
		snprintf(http_header, sizeof(http_header), "\nHTTP/1.1 200 OK\r\n"
		             "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
		             "Date: %s\r\n"
		             "Content-Type: text/html\r\n"
		             "Content-Length: %ld\r\n"
		             "Connection: Closed\r\n\r\n",
		             get_current_date(date, sizeof(date)),
		             strlen(buffer));

		strcat(http_header, buffer);
		send(client_socket_fd, http_header, sizeof(http_header), 0);
		printf("File \"%s\" has been sent to client\n", dir);

		memset(http_header, '\0', sizeof(http_header));
		memset(dir, '\0', sizeof(dir));
		memset(path, '\0', sizeof(path));

        pthread_mutex_lock(&mtx);
		close(client_socket_fd);
		free(p);
		pthread_mutex_unlock(&mtx);
    }

    pthread_exit(NULL);
}


char *get_current_date(char *str, int len)
{
    time_t t = time(NULL);
    struct tm res;

    strftime(str, len, RFC1123FMT, localtime_r(&t, &res));
    return str;
}

