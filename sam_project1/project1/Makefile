# compiler and flags 
CC = gcc
CFLAGS = -Wall -g -I.

# targets 
all: client server 

client: client.c
	$(CC) $(CFLAGS) -o client client.c

server: server.c
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm -f client server *.o

.PHONY: all clean