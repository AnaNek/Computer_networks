#include <stdio.h>	//printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#define BUF_SIZE 512	//Max length of buffer
#define PORT 8888	//The port on which to listen for incoming data

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

void translate(int num, int divider)
{
    int index = 0;
    int remainder = 0;
    char result[BUF_SIZE];
    
    while (num)
    {
        remainder = num % divider;
        if (remainder >= 10)
        {
            result[index] = remainder + 55;
        }
        else
        {
            result[index] = remainder + 48;
        }
        index++;
        num /= divider;
    }
    
    printf("%d-numeral system:", divider); 
    for (int i = index - 1; i >= 0; i--)
    {
        printf("%c", result[i]);
    }
    printf("\n");
}

int main(void)
{
	struct sockaddr_in si_me, si_other;
	
	int i, slen = sizeof(si_other) , recv_len;
	char buf[BUF_SIZE];
	int num = 0;
	
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		finish("socket");
	}
	
	//signal(SIGINT, catch_sigint);
	
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
	    finish("bind");
	}
	
	//keep listening for data
	//while(1)
	//{
	printf("Waiting...\n");
	fflush(stdout);
		
	//try to receive some data, this is a blocking call
	if ((recv_len = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &si_other, &slen)) == -1)
	{
	    finish("recvfrom()");
	}
		
	//print details of the client/peer and the data received
	printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
	
	num = *(int*)buf;		
	printf("Received: %d\n" , num);
	
	translate(num, 2);
	translate(num, 8);
	translate(num, 15);
	translate(num, 16);	
	//now reply the client with the same data
	if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
	{
		finish("sendto()");
	}
	//}

	close(s);
	return 0;
}
