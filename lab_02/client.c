#include <stdio.h>	//printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#define SERVER "127.0.0.1"
#define BUF_SIZE 512	
#define PORT 8888	//The port on which to send data

void finish(char *er)
{
	perror(er);
	exit(1);
}

void input_integer(long int *n)
{
    char *p;
    char message[BUF_SIZE];
    printf("Enter integer : ");
	while (fgets(message, sizeof(message), stdin)) 
	{
        *n = strtol(message, &p, 10);
        if (p == message || *p != '\n') 
        {
            printf("Please enter an integer: ");
        } else break;
    }
}

int main(void)
{
    int s;
	struct sockaddr_in si_other;
	int i, slen=sizeof(si_other);
	char buf[BUF_SIZE];
	long int n;

	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		finish("socket() error");
	}
    
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	
	if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

    input_integer(&n);
		
	if (sendto(s, &n, sizeof(n) , 0 , (struct sockaddr *) &si_other, slen)==-1)
	{
		finish("sendto()");
	}
		
	memset(buf,'\0', BUF_SIZE);
	if (recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &si_other, &slen) == -1)
	{
		finish("recvfrom()");
	}
		
	printf("Server received %d\n", *(int*)buf);

	close(s);
	return 0;
}
