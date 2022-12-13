#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

int sockfd;
struct sockaddr_in server_addr;
socklen_t server_addr_len;

time_t td;
struct tm tm;

char buf[1024];

int main(int args, char *argv[]) {

  if (3 != args) {
    printf("Parameter error\n\n\tSample: %s 127.0.0.1 50000\n", argv[0]);
    exit(1);
  }

  inet_aton(argv[1], &server_addr.sin_addr);
  server_addr.sin_family = PF_INET;
  server_addr.sin_port = htons(atoi(argv[2]));

  if (-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0))) {
    perror("socket");
    exit(1);
  }

  if (-1 ==
      connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
    perror("connect");
    exit(1);
  }
  printf("Connected to %s\n", inet_ntoa(server_addr.sin_addr));

  int recv_len;

  while (1) {
    printf("Message: ");
    memset(buf, 0, sizeof buf);
    scanf("%s", buf);

    if (0 == strncmp(buf, "quit", 4)) {
      printf("connect close\n");
      close(sockfd);
      return 0;
    }

    time(&td);
    tm = *localtime(&td);

    write(sockfd, buf, strlen(buf));
    read(sockfd, buf, sizeof buf);
    printf("Send successfully\n");
  }
  return 0;
}