CC=gcc
CFLAGS=-Wall

server: server.c
	$(CC) $(CFLAGS) -o serverside server.c

client: client.c
	$(CC) $(CFLAGS) -o clientside client.c

all: server client

clean:
	rm -f clientside serverside
