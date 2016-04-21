#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define EPOLL_RUN_TIMEOUT -1
#define EPOLL_SIZE 10
#define BUF_SIZE 1024
#define CMD_EXIT "exit"

// chat message buffer
char message[BUF_SIZE];

int main()
{
        int sock, pid, pipe_fd[2], epfd; //pipe_fd хранит файловые дескрипторы для чтения из pipe_fd[0]  и запись в pipe_fd[1]
        int continue_to_work = 1;

        sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock < 0)
        {
            perror("Socket init error");
            return 1;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(3000);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("Connection error");
            return 1;
        }

        struct epoll_event ev, events[2]; // Socket(in|out) & Pipe(in)// для хранения событий от epoll_wait(events), всего 2 события: 'sock' и 'pipe' от "родительского процесса"
        ev.events = EPOLLIN | EPOLLET;
        
        if(pipe(pipe_fd) < 0)
        {
            perror("Pipe error");
            return 1;
        }

        epfd = epoll_create(EPOLL_SIZE);

        if(epfd < 0)
        {
            perror("Epoll create error");
            return 1;
        }

        ev.data.fd = sock;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) < 0)
        {
            perror("epoll_ctl (socket) error");
            return 1;
        }
        // добавили сокет, то есть наблюдаем за ним

        ev.data.fd = pipe_fd[0];
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipe_fd[0], &ev) < 0 )
        {
            fprintf(stderr, "epoll_ctl(pipe)Error\n");
            return 1;
        }
        // отслеживаем сообщения от дочернего процесса(который ждет сообщения от сервера)

        pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "ForkError\n");
            return 1;
        }
        switch(pid)
        {
                case 0: // "дочерний" процесс
                       //printf("%d(child) started\n", getpid());
                        close(pipe_fd[0]); 
                        while(continue_to_work)
                        {
                                bzero(&message, BUF_SIZE);
                                fgets(message, BUF_SIZE, stdin);
                                if(strncasecmp(message, CMD_EXIT, strlen(CMD_EXIT)) == 0)
                                {
                                        continue_to_work = 0;
                                        // 
                                } 
                                else if (write(pipe_fd[1], message, strlen(message) - 1) < 0)
                                {
                                    perror("Pipe write error");
                                    return 1;
                                }
                        }
                        break;
                default: // родительский процесс
                //printf("%d(parent) started\n", getpid());
                        close(pipe_fd[1]); 

                        //  epoll_wait's events count(epoll_events_count)
                        // результат разных функций(res)
                        int epoll_events_count, res;

                        while(continue_to_work) {
                                // 2 - потому что у нас есть 2 сообщения : от сервера и от дочернего процесс
                                epoll_events_count = epoll_wait(epfd, events, 2, EPOLL_RUN_TIMEOUT);
                        
                                if(epoll_events_count < 0)
                                {
                                    fprintf(stderr, "Epoll_Wait_Error\n");
                                    return 1;
                                }
                                for(int i = 0; i < epoll_events_count ; i++){
                                        bzero(&message, BUF_SIZE);
                                        if(events[i].data.fd == sock){
                                                res = recv(sock, message, BUF_SIZE, 0);
                                                if (res < 0)
                                                {
                                                    perror("Receive error");
                                                    return 1;
                                                }

                                                // 0 - значит закрываем соединение
                                                if (res == 0)
                                                {
                                                    close(sock);
                                                    continue_to_work = 0;
                                                } 
                                                else
                                                {
                                                    printf("%s\n", message);
                                                }
                                                
                                        } 
                                        else
                                        {
                                                res = read(events[i].data.fd, message, BUF_SIZE);
                                                if (res < 0)
                                                {
                                                    fprintf(stderr, "Read_Error\n");
                                                    return 1;
                                                }

                                                // 0 - значит дочерний процесс собирается прекратить
                                                if(res == 0) 
                                                {
                                                    continue_to_work = 0; 
                                                }
                                                // exit to parent to send message to server
                                                else
                                                {
                                                        if (send(sock, message, BUF_SIZE, 0) < 0)
                                                        {
                                                            fprintf(stderr, "Send_Error");
                                                            return 1;
                                                        }
                                                }
                                        }
                                }
                        }
        }
        if(pid)
        {
                close(pipe_fd[0]);
                close(sock);
        } else 
        {
                close(pipe_fd[1]);
        }

        return 0;
}