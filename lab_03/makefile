all: myhttpd client
myhttpd: server.c server.h list_fd.c list_fd.h
	gcc -o myhttpd server.c server.h list_fd.c list_fd.h -pthread -g
client: client.c
	gcc -o client client.c
clean:
	-rm -f myhttpd client
