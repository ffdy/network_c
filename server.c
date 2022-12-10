#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

pid_t sub_pid;

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

  int do_len;
  printf("Start to be connected\n");

  while (1) {
    if (-1 == (newfd = accept(sockfd, (struct sockaddr *)&client_addr,
                              &client_addr_len))) {
      perror("accept");
      exit(1);
    }

    sub_pid = fork();
    if (0 == sub_pid) {
      while (1) {
        do_len = read(newfd, buf, sizeof(buf));
        if (do_len == 0) {
          printf("connect from %s close\n", inet_ntoa(client_addr.sin_addr));
          close(newfd);
          return 0;
        }
        printf("Message from %s : %s\n", inet_ntoa(client_addr.sin_addr), buf);

        write(newfd, "ACK", 3);
      }
    } else if (-1 == sub_pid) {
      perror("fork");
      exit(1);
    } else {
      close(newfd);
    }
  }

  return 0;
}