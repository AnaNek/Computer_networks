#include "server.h"

void catch_sigint(int signum)
{
    free_list(&head);
    close(server_socket_fd);
    exit(1);
}

int main(int argc, char *argv[])
{

    int opt = 0;
    int serving_port = 0;
    int num_of_threads = 0;
    int client_socket_fd;
    int flags = 0;
    struct sockaddr_in address, caddr;
    socklen_t clientlen;
    node_t *list_node;
    fd_set set;
    head = NULL;
        
    shutd = 0;
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

    // Создание пула потоков
    for (int i = 0 ; i < num_of_threads ; i ++)
    {
        pthread_create(&thread_ids[i], NULL, client_handler, &head);
    }
    
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("Error creating socket");
        return EXIT_FAILURE;
    }
    
    signal(SIGINT, catch_sigint);
    
    clientlen = sizeof(caddr);
    
    // Описание адреса сервера
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(serving_port);
    
    flags = fcntl(server_socket_fd, F_GETFL, 0);
    fcntl(server_socket_fd, F_SETFL, flags | O_NONBLOCK);
    
    FD_ZERO(&set);
    FD_SET(server_socket_fd, &set);
         
    printf("ADDRESS %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    
    // Пивязка сокета к адресу
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
        
    while (1)
    {
        sleep(1);
        puts("Waiting for connections...");

        if (select(server_socket_fd + 1, &set, NULL, NULL, NULL) <= 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
           
        if (shutd)
        { 
            break;
        }
             
        if (FD_ISSET(server_socket_fd, &set))
        {      
            clientlen = sizeof(caddr);
            if ((client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &caddr, &clientlen)) < 0)
            {
                perror("Error in accept");
                exit(EXIT_FAILURE);
            }
            getsockname(client_socket_fd, (struct sockaddr *) &caddr, &clientlen);
            printf("Client connected to port: %d\n", ntohs(caddr.sin_port));
        }
        
        pthread_mutex_lock(&mtx);

        push(&head, client_socket_fd);
        
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mtx);
        
    }

    for (int i = 0 ; i < num_of_threads ; i ++)
    {
        pthread_join(thread_ids[i], NULL);
    }
    
    free_list(&head);
    close(server_socket_fd);
    
    return 0;
}

void *client_handler(void *arg)
{
    node_t *p;
    char buffer[BUFFER_SIZE];
    
    int client_socket_fd;
    char dir[1024];
    char http_header[BUFFER_SIZE];
    char request[1024];
    char path[1024];
    char method[3];
    int fd;
    int total_pages = 0, total_bytes = 0;
    node_t **list_head = NULL;
      
    while (1)
    {
        if (shutd)
        {
            pthread_exit(NULL);
        }
        printf("Thread %lu \n", pthread_self());
        pthread_mutex_lock(&mtx);
		    
        list_head = (node_t **)arg;
        while (*list_head == NULL)
        {
            pthread_cond_wait(&cond, &mtx);
        }
        p = pop(list_head);
        client_socket_fd = p->fd;
        
        printf("Thread %lu started...\n", pthread_self());
        pthread_mutex_unlock(&mtx);
        
        recv(client_socket_fd, &request, sizeof(request), 0);
        
        sscanf(request,"%s%s", method, path);
        
        update_stat();
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
            if ((fd = open("stats" , O_RDONLY)) != -1)
            {
                read(fd, http_header, sizeof(http_header));
                close(fd);
            }
            send(client_socket_fd, http_header, sizeof(http_header), 0);
            memset(http_header, '\0', sizeof(http_header));
            memset(path, '\0', sizeof(path));
            memset(method, '\0', sizeof(method));
            continue;
        }
        else if (!strcmp(method, "SHUTDOWN"))
        {
            printf("Terminating server...\n");
            pthread_mutex_lock(&mtx);
            close(client_socket_fd);
            free(p);
            shutd = 1;
            shutdown(server_socket_fd, 2);
            pthread_mutex_unlock(&mtx);
            pthread_exit(NULL);
        }
        strcpy(dir, root_dir);
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
            pthread_mutex_lock(&mtx);
            close(client_socket_fd);
            free(p);
            pthread_mutex_unlock(&mtx);
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

void update_stat(void)
{
    char str[1024] = {0};
    time_t t = time(NULL);
    FILE *f = NULL;
    struct tm res;
    static int days[7] = {0};
    static int hours[24] = {0};
    static int total_requests = 0;
    char buf[1024];
    const char *day_names[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    localtime_r(&t, &res);
    
    hours[res.tm_hour] += 1;
    days[res.tm_wday] += 1;
    total_requests++;
    
    snprintf(buf, sizeof(buf), "HOUR  COUNT OF REQUESTS  PERS OF REQUESTS\n");
    strcat(str, buf);
    memset(buf, '\0', sizeof(buf));
    for (int i = 0; i < 24; i++)
    {
        snprintf(buf, sizeof(buf), "%d            %d          %g %% \n", i, hours[i], hours[i] / (double)total_requests * 100);
        strcat(str, buf);
        memset(buf, '\0', sizeof(buf));
    }
    
    snprintf(buf, sizeof(buf), "DAY  COUNT OF REQUESTS  PERS OF REQUESTS\n");
    strcat(str, buf);
    memset(buf, '\0', sizeof(buf));
    
    for (int i = 0; i < 7; i++)
    {
        snprintf(buf, sizeof(buf), "%s            %d           %g %% \n", day_names[i], days[i], days[i] / (double)total_requests * 100);  
        strcat(str, buf);
        memset(buf, '\0', sizeof(buf));
    }
    
    f = fopen("stats", "w");
    if (f)
    { 
      fprintf(f, "%s\n", str);
    }
    fclose(f);
}

