#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <list>
#include <time.h>
#include <string.h>

#define EPOLL_SIZE 100
#define BUF_SIZE 1024
#define EPOLL_RUN_TIMEOUT -1
#define STR_MESSAGE "Client #%d>> %s"
#define STR_NOONE_CONNECTED "No one connected to server except you!"
#define STR_WELCOME "Welcome to seChat! You ID is: Client #%d"

using namespace std;
//лист сокет-клиентов
static list<int> clients_list;

int setnonblocking(int sockfd)
{
  
   if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) < 0)
    {
      fprintf(stderr,"setnonblockingerror\n");
      return -1;
    }
   return 0;

}
// *** Handle incoming message from clients
int handle_incoming_message(int client)
{
   // получает сообщение от клиента
   // и формат сообщения для заполнения
   char buf[BUF_SIZE];
   bzero(buf, BUF_SIZE);
   //держит различные результаты
   int len;
   len = recv(client, buf, BUF_SIZE, 0);
   if (len < 0)
   {
    fprintf(stderr, "RecvError\n");
   }
  // 0 - клиент закончил соединение
   if(len == 0)
   {
       close(client);
       clients_list.remove(client);
   } 
   else 
   {
      printf("Client #%d: %s\n", client, buf); 
   }

   return len;
}




int main()
{
   //     listener главного сервера
   int listener;

   // define ip & ports for server(addr)
   //     and incoming client ip & ports(their_addr)
   struct sockaddr_in addr, their_addr;
   
   addr.sin_family = AF_INET;

   addr.sin_port = htons(3000);
   
   addr.sin_addr.s_addr = htonl(INADDR_ANY);

   //   размер адреса
   socklen_t socklen;
   socklen = sizeof(struct sockaddr_in);

   //     event template for epoll_ctl(ev)
   //     storage array for incoming events from epoll_wait(events)
   //     and maximum events count could be EPOLL_SIZE
   struct epoll_event ev, events[EPOLL_SIZE];
   
   ev.events = EPOLLIN | EPOLLET;

   char message[BUF_SIZE];

   //     epoll descriptor для просмотра событий
   int epfd;

   //     дескриптор нового клиента
   //     держит результаты различных функций(res)
   //     держит epoll_wait's events count(epoll_events_count)
   int client, res, epoll_events_count;

   listener = socket(AF_INET, SOCK_STREAM, 0);
   if (listener < 0)
   {
    perror("SocketError");
    return 1;
   }
  //    настраиваем nonblocking socket
   setnonblocking(listener);

   if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
   {
    perror("BindError");
    return 1;
   }
   // начинаем слушать соединения
   if (listen(listener, 1) < 0)
   {
    perror("ListenError");
    return 1;
   }
   // *** Setup epoll
   //     create epoll descriptor
   //     and backup store for EPOLL_SIZE of socket events
   epfd = epoll_create(EPOLL_SIZE);
   if (epfd < 0)
   {
    fprintf(stderr, "Epoll_create_Error\n");
    return 1;
   }

   //     set listener to event template
   ev.data.fd = listener;
  //     add listener to epoll
   if (epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &ev) < 0)
   {
    fprintf(stderr, "Epoll_ctl_Error\n");
    return 1;
   }
   

   ev.data.fd = STDIN_FILENO;
   if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0)
   {
    fprintf(stderr, "Epoll_ctl_Error\n");
    return 1;
   }
     // *** Main cycle(epoll_wait)
     while(1)
     {
         epoll_events_count = epoll_wait(epfd, events, 1, EPOLL_RUN_TIMEOUT);
         if (epoll_events_count < 0)
         {
          fprintf(stderr, "Epoll_wait_Error\n");
          return 1;
         }
         for(int i = 0; i < epoll_events_count ; i++)
         {
            //новое клиентское соединение
            if(events[i].data.fd == listener)
            {
                client = accept(listener, (struct sockaddr *) &their_addr, &socklen);
                if(client < 0)
                {
                  fprintf(stderr, "AcceptError\n");
                  return 1;
                }
                        
            // настраиваем nonblocking socket
                setnonblocking(client);
            // добавляем нового клиента to event template
                ev.data.fd = client;
            // добавляем нового клиента to epoll
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev) < 0)
                {
                  fprintf(stderr, "Epoll_ctlError\n");
                  return 1;
                }
                clients_list.push_back(client); // добавляем новое соединение в лист клиентов
                printf("added %d client\n", client);
            // send initial welcome message to client
                bzero(message, BUF_SIZE);
                res = sprintf(message, STR_WELCOME, client);
                res = send(client, message, BUF_SIZE, 0);
                if (res < 0)
                {
                  fprintf(stderr, "SendError\n");
                }
            } else if (events[i].data.fd == STDIN_FILENO)
            {
              char* message_to_send;
               while (scanf("%ms", &message_to_send))
              {
              
              for (auto it = clients_list.begin(); it != clients_list.end(); it++)
              {
                res = send(*it, message_to_send, strlen(message_to_send), 0);
                if (res < 0)
                {
                  perror("send message to client error");
                    return 1;
                }
                //printf("sent message %s to %d\n", message_to_send, *it);
              }
              }
            free(message_to_send);
            }
            else 
            { // EPOLLIN event for others(new incoming message from client)
                res = handle_incoming_message(events[i].data.fd);
                if (res < 0)
                {
                  perror("handle_incoming_messageError");
                  return 1;
                }
            }
         }
     }
  
   close(listener);
   close(epfd);

   return 0;
}
