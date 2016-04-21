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
#include <sys/un.h>

#define SHMEM_PATH "/my_shmem"
// 64 KiB
#define SHMEM_SIZE (1 << 16)

#define S_PATH "Ficus"

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

    //int new_val; 
    //printf("Enter the value to put in the memory\n");
    //scanf("%d", &new_val);
	//*(int *) ptr = new_val;
    //printf("New Value in the memory!\n");

	int sock;
	//char c;
	sock = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sock < 0)
	{
		perror("ErrorOfDescrip");
		return 1;
	}
	
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	//addr.sin_port = htons(3000);
	//addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    strcpy(addr.sun_path, S_PATH);


	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("ErrorOfConnection");
		return 1;
	}

    int flag = 1;

	while (flag)
	{  
        int new_val; 
        printf("Enter the value to put in the memory and enter the message(If you want to close server enter 'Exit' and enter 'Ficus' to close client).\n");
        scanf("%d\n", &new_val);
        *(int *) ptr = new_val;
        printf("New Value in the memory!\n");
		//char c = fgetc(stdin);
		char buf[256];
		if (fgets(buf, sizeof(buf)/sizeof(char) ,stdin) == NULL)
			break;
		send(sock, buf, sizeof(buf), 0);
        char answer[4];
        sscanf(buf, "%s", answer);
        if(strcmp(answer,"Ficus") == 0)
        {
            flag = 0;
            close(sock);

            res = close(shmem_fd);
            if (res) 
            {
                perror("close");
                exit(1);
            }
        return 0;
        }

		
 	
	}

	close(sock);

	 res = close(shmem_fd);
    if (res) 
    {
        perror("close");
        exit(1);
    }
    
    res = shm_unlink(SHMEM_PATH);
    if (res) 
    {
        perror("shm_unlink");
        exit(1);
    }
	
 
	return 0;
}
