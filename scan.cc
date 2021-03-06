#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <map>

#define MAX_EVENTS (1 << 16)

void setnonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

enum State {
  kStateConnecting = 0,
  kStateWriting
};

struct SocketState {
  uint32_t addr;
  State state;

  SocketState() : addr(0), state(kStateConnecting) {}
};

// 10.55.192.192
#define ADDR_START 171425984
// 10.55.192.255
#define ADDR_FINISH 171426047

int main() {
  
  struct sockaddr_in addr_in;
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(80);
  int epollfd = epoll_create(1);
  struct epoll_event ev;  
  struct epoll_event events[MAX_EVENTS];  

  std::map<int, SocketState> socks;

  const char* HTTP_GET = "GET / HTTP/1.1\r\n\r\n";
  const int HTTP_GET_SIZE = strlen(HTTP_GET);

  uint32_t addr = ADDR_START;
  while (addr < ADDR_FINISH || socks.size()) {
    while (addr < ADDR_FINISH && socks.size() < MAX_EVENTS) {
      // register new socket
	  int sock = socket(AF_INET, SOCK_STREAM, 0);
	  setnonblocking(sock);
	  addr_in.sin_addr.s_addr = addr;
	  if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr_in), sizeof(addr_in))) {
        if (errno != EINPROGRESS) {
		  perror("connect");
		  addr++;
          continue;
        }
	  }
	  ev.events = EPOLLOUT | EPOLLET;
	  ev.data.fd = sock;
	  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev)) {
		perror("epoll_ctl: sock");
		exit(EXIT_FAILURE);
	  }
	  socks[sock].addr = addr;
      addr++;
    }

    int events_num = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    for (int i = 0; i < events_num; i++) {
      int sock = events[i].data.fd;
      SocketState& state = socks[sock]; 
      if (events[i].events & EPOLLET) {
        // error
        printf("IP %d: error\n", state.addr);
        socks.erase(sock);
        close(sock);
      }        
      switch (state.state) {
		case kStateConnecting:
          // ready to write
          write(sock, HTTP_GET, HTTP_GET_SIZE);
          // change epoll type to EPOLLIN
          ev.events = EPOLLIN;
          if (epoll_ctl(epollfd, EPOLL_CTL_MOD, sock, &ev)) {
		    perror("epoll_ctl: modify sock");
		    exit(EXIT_FAILURE);
		  }
          state.state = kStateWriting;
		  break;
        case kStateWriting: {
          // ready to read server's response
          char buf[1024] = {0};
          int rsize;
          printf("IP %d:\n", state.addr);
          while ((rsize = read(sock, buf, sizeof(buf)))) {
            if (rsize < 0) {
              perror("read");
              break;
            }
            write(STDOUT_FILENO, buf, rsize);
          }
          // unregister from epoll
          if (epoll_ctl(epollfd, EPOLL_CTL_DEL, sock, NULL)) {
		    perror("epoll_ctl: modify sock");
		    exit(EXIT_FAILURE);
		  }
          socks.erase(sock);
          close(sock);
          break;
        }
        default:
          break;
      } // switch
    } // epoll events for
  } // while
  close(epollfd);
  return 0;
}
