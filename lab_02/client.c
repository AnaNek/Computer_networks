#include <stdio.h>	//printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#define SERVER "127.0.0.1"
#define BUF_SIZE 512	//Max length of buffer
#define PORT 8888	//The port on which to send data

int s;

void finish(char *er)
{
	perror(er);
	exit(1);
}

/*
void catch_sigint(int signum)
{
    close(s);
    perror("\nClose by Ctrl+C\n");
    exit(1);
}
*/

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
	struct sockaddr_in si_other;
	int i, slen=sizeof(si_other);
	char buf[BUF_SIZE];
	long int n;

	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		finish("socket() error");
	}

    //signal(SIGINT, catch_sigint);
    
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	
	if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	//while(1)
	//{

    input_integer(&n);
		
	//send the message
	if (sendto(s, &n, sizeof(n) , 0 , (struct sockaddr *) &si_other, slen)==-1)
	{
		finish("sendto()");
	}
		
	//receive ah reply and print it
	//clear the buffer by filling null, it might have previously received data
	memset(buf,'\0', BUF_SIZE);
	//try to receive some data, this is a blocking call
	if (recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &si_other, &slen) == -1)
	{
		finish("recvfrom()");
	}
		
	printf("Server received %d\n", *(int*)buf);
	//}

	close(s);
	return 0;
}
