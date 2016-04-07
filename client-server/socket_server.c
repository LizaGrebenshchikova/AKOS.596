#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>


int main()
{
	int sock, listener;
	struct sockaddr_in addr;
    int bytes_read;

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0)
	{
		fprintf(stderr,"SocketError\n");
		return 1;
	}

	addr.sin_family = AF_INET;
    addr.sin_port = htons(3000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

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
        while(1)
        {
            bytes_read = recv(sock, &s, sizeof(char), 0);
            
            if(bytes_read <= 0) break;
            
            fputc(s, file); 
            printf("%c", s);
        }
           
    	fclose(file);
    	close(sock);
    }
    return 0;
 }
