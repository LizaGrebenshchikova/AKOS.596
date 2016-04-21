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
#include <stdlib.h>

#define EPOLL_SIZE 100
#define BUF_SIZE 1024
#define EPOLL_RUN_TIMEOUT -1
#define STR_MESSAGE "Client #%d>> %s"
#define STR_NOONE_CONNECTED "No one connected to server except you!"
#define STR_WELCOME "Welcome to seChat! You ID is: Client #%d"
//лист сокет-клиентов
list<int> clients_list;

int main()
{
       //     listener главного сервера
       int listener;
      
       // define ip & ports for server(addr)
       //     and incoming client ip & ports(their_addr)
       struct sockaddr_in addr, their_addr;
       
       addr.sin_family = AF_INET;
       addr.sin_port = htons(3000);
       addr.sin_addr.s_addr = inet_addr(INADDR_ANY);

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

       // для подсчета время выполнения программы
       clock_t tStart;

       //     дескриптор нового клиента
       //     держит результаты различных функций(res)
       //     держит epoll_wait's events count(epoll_events_count)
       int client, res, epoll_events_count;

       listener = socket(AF_INET, SOCK_STREAM, 0);
       if (listener < 0)
       {
        fprintf(stderr,"Listener_Error\n");
        return 1;
       }
      //    настраиваем nonblocking socket
       setnonblocking(listener);

       if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
       {
        fprintf(stderr, "Bind_Error\n");
        return 1;
       }
       // начинаем слушать соединения
       if (listen(listener, 1) < 0)
       {
        fprintf(stderr,"Listener_Error\n");
        return 1;
       }
       // *** Setup epoll
       //     create epoll descriptor
       //     and backup store for EPOLL_SIZE of socket events
       epfd = epoll_create(EPOLL_SIZE));
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
       // *** Main cycle(epoll_wait)
       while(1)
       {
           epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_RUN_TIMEOUT);
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
              // send initial welcome message to client
                  bzero(message, BUF_SIZE);
                  res = sprintf(message, STR_WELCOME, client);
                  res = send(client, message, BUF_SIZE, 0));
                  if (res < 0)
                  {
                    fprintf(stderr, "SendError\n");
                  }

              } else 
              { // EPOLLIN event for others(new incoming message from client)
                  res = handle_message(events[i].data.fd);
                  if (res < 0)
                  {
                    fprintf(stderr, "handle_messageError\n");
                    return 1;
                  }
              }
           }
       }

       close(listener);
       close(epfd);

       return 0;
   }

   // *** Handle incoming message from clients
   int handle_message(int client)
   {
       // получает сообщение от клиента
       // и формат сообщения для заполнения
       char buf[BUF_SIZE], message[BUF_SIZE];
       bzero(buf, BUF_SIZE);
       bzero(message, BUF_SIZE);
      //держит различные результаты
       int len;
       len = recv(client, buf, BUF_SIZE, 0);
       if (len < 0)
       {
        fprintf(stderr, "RecvError\n");
       }
      // 0 - клиент закончил соединение
       if(len == 0){
           close(client);
           clients_list.remove(client);
       } else 
       {
        if(clients_list.size() == 1) 
        { // никто ,кроме тебя не присоединился к серверу
          if (send(client, STR_NOONE_CONNECTED, strlen(STR_NOONE_CONNECTED), 0) < 0)
          {
            fprintf(stderr,"SendError\n");
            return -1;
          }
          return len;
        }
        
        sprintf(message, STR_MESSAGE, client, buf);

        // populate message around the world ;-)...
        list<int>::iterator it;
        for(it = clients_list.begin(); it != clients_list.end(); it++)
        {
          if(*it != client)
          { // ... except youself of course
            if (send(*it, message, BUF_SIZE, 0) < 0 )
            {
              fprintf(stderr, "SendError\n");
              return -1;
            }
          }
        }
          
       }

       return len;
   }