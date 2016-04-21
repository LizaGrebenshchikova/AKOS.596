#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
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

    *((int *)ptr) = 69;
    int value = *((int *)ptr);


	int sock, listener;
	struct sockaddr_un addr;
    int bytes_read;

	listener = socket(AF_UNIX, SOCK_STREAM, 0);
	if(listener < 0)
	{
		fprintf(stderr,"SocketError\n");
		return 1;
	}

	addr.sun_family = AF_UNIX;
    //addr.sin_port = htons(3000);
    //addr.sin_addr.s_addr = htonl(INADDR_ANY);
    strcpy(addr.sun_path, S_PATH);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
    	fprintf(stderr,"BindError\n");
    	return 1;
    }

    listen(listener, 1);

    while(1)
    {   

    	sock = accept(listener, NULL, NULL); // NULL,NULL - значит адрес клиента не интересует,принимает все
    	if (sock < 0)
    	{
    		fprintf(stderr,"AcceptError\n");
    		return 1;
    	}
        FILE *file = fopen("log","a");
        char s;
        //int value = *((int *)ptr);
        while(1)
        {   
            printf("Value in the memory: %d\n", value);
            value = *((int *)ptr);
            char buf[256];
            bytes_read = recv(sock, buf, sizeof(buf) / sizeof(char), 0);
            
            if(bytes_read <= 0) break;
            
            fprintf(file,"%s", buf); 
            printf("%s", buf);
            value = *((int *)ptr);
            
            char answer[4];
            sscanf(buf,"%s",answer);
            if (strcmp(answer,"Exit") == 0)
            {
                fclose(file);
                close(sock);
                if (res) 
                {
                    perror("close");
                    exit(1);
                }
    
                unlink(S_PATH);
                return 0;
            }
        }
           
    	fclose(file);
    	close(sock); 
    }
    
    if (res) 
    {
        perror("close");
        exit(1);
    }
    
    unlink(S_PATH);
  return 0;
 }
