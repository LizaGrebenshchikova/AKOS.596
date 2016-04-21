CC = gcc
CFLAGS = -O3 -Wall -Wextra -pedantic

all: client.o fork.o #thread.o pull_thread.o server.o

fork.o: fork_server.o client.o

client.o:
	$(CC) socket_client.c $(CFLAGS) -o client

fork_server.o:
	$(CC) fork_server.c $(CFLAGS) -o fork_server

clean:
	rm -rf server client fork_server log *.o* || 1


#server.o:
#	$(CC) socket_server.c $(CFLAGS) -o server
