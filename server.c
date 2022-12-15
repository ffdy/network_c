#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// 缺少一个链表管理断开的连接

#define MAX(sockfd, nfds) (sockfd > nfds ? sockfd : nfds)

int sockfd, newfd;
struct sockaddr_in server_addr, client_addr;
socklen_t server_addr_len, client_addr_len;

char buf[1024];

int main(int args, char *argv[]) {

  if (2 != args) {
    printf("Parameter error\n\n\tSample: %s %s\n", argv[0], "50000");
    exit(1);
  }

  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_family = PF_INET;
  server_addr.sin_port = htons(atoi(argv[1]));

  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (-1 == sockfd) {
    perror("socket");
    exit(1);
  }

  if (-1 ==
      bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
    perror("bind");
    exit(1);
  }

  if (-1 == listen(sockfd, 128)) {
    perror("listen");
    exit(1);
  }

  printf("Start to be connected\n");
  int do_len, nfds = -1;

  int fd_array[1024], fd_array_len = 0;
  fd_set readset, old_readset;
  FD_ZERO(&old_readset);
  FD_SET(sockfd, &old_readset);
  nfds = MAX(sockfd, nfds);

  while (1) {
    readset = old_readset;
    select(nfds + 1, &readset, NULL, NULL, NULL);

    if (FD_ISSET(sockfd, &readset)) {
      if (-1 == (newfd = accept(sockfd, (struct sockaddr *)&client_addr,
                                &client_addr_len))) {
        perror("accept");
        exit(1);
      }
      printf("Connect from %s\n", inet_ntoa(client_addr.sin_addr));

      FD_SET(newfd, &old_readset);
      nfds = MAX(nfds, newfd);
      fd_array[fd_array_len++] = newfd;
    }

    for (int i = 0; i < fd_array_len; i++) {
      if (FD_ISSET(fd_array[i], &readset)) {
        do_len = read(fd_array[i], buf, sizeof(buf));
        if (do_len == 0) {
          // 连接关闭
          printf("connect from %s close\n", inet_ntoa(client_addr.sin_addr));
          close(fd_array[i]);
          FD_CLR(fd_array[i], &old_readset);
          continue;
        }
        printf("Message from %s : %s\n", inet_ntoa(client_addr.sin_addr), buf);

        write(fd_array[i], "ACK", 3);
      }
    }
  }

  return 0;
}