all: myhttpd client
myhttpd: server.c server.h
	gcc -o myhttpd server.c server.h -pthread -g
client: client.c
	gcc -o client client.c
clean:
	-rm -f myhttpd client