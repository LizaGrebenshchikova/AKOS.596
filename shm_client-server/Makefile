CC=g++

all: shm_client shm_server

shm_client: shm_client.cc
	$(CC)  shm_client.cc -lrt -o shm_client

shm_server: shm_server.cc
	$(CC)  shm_server.cc -lrt -o shm_server

clean:
	rm -rf shm_client shm_server