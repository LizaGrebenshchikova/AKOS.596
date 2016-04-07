#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>

int main()
{
	int sock;
	//char c;
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0)
	{
		fprintf(stderr,"ErrorOfDescrip\n");
		return 1;
	}
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(3000);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		fprintf(stderr,"ErrorOfConnection\n");
		return 1;
	}

	while (1)
	{
		char c = fgetc(stdin);
		if (c == EOF)
			break;
		send(sock, &c, sizeof(c), 0);	
	}
	
	
    close(sock);
	return 0;
}
