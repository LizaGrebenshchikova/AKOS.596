#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define SHMEM_PATH "/my_shmem"
// 64 KiB
#define SHMEM_SIZE (1 << 16)

int main()
{
	int res, shmem_fd;
    void *ptr;
    shmem_fd = shm_open(SHMEM_PATH, O_RDWR | O_CREAT, 0666);
    
    if (shmem_fd == -1) 
    {
        perror("shm_open");
        exit(1);
    }

    res = ftruncate(shmem_fd, SHMEM_SIZE);

    if (res)
    {
        perror("ftruncate");
        exit(1);
    }

    ptr = mmap(NULL, SHMEM_SIZE,
             PROT_READ | PROT_WRITE,
             MAP_SHARED, shmem_fd, 0);
    if (!ptr) 
    {
        perror("mmap");
        exit(1);
    }

    int val = *(int *) ptr;
    printf("Current value in the memory: %d\n", val);

    int new_val; 
    printf("Enter the value to put in the memory\n");
    scanf("%d", &new_val);
	*(int *) ptr = new_val;
    printf("New Value in the memory!\n");

	int sock;
	//char c;
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0)
	{
		perror("ErrorOfDescrip");
		return 1;
	}
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(3000);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("ErrorOfConnection");
		return 1;
	}


	while (1)
	{
		//char c = fgetc(stdin);
		char buf[256];
		if (fgets(buf, sizeof(buf)/sizeof(char) ,stdin) == NULL)
			break;
		send(sock, buf, sizeof(buf), 0);
		
 	
	}

	close(sock);
	
 
	return 0;
}
