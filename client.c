#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <time.h>
#include <unistd.h>

int sockfd;
struct sockaddr_in server_addr;
socklen_t server_addr_len = sizeof(server_addr);

time_t td;
struct tm tm;

char buf[1024];

void sigio_handler(int signo) {
  while (-1 != recvfrom(sockfd, buf, sizeof(buf), 0,
                        (struct sockaddr *)&server_addr, &server_addr_len)) {
    printf("Recv successfully: %s\n", buf);
  }
  perror("recvfrom");
}

int main(int args, char *argv[]) {

  struct sigaction action;
  bzero(&action, sizeof(action));
  action.sa_handler = sigio_handler;
  if (-1 == sigaction(SIGIO, &action, NULL)) {
    perror("sigaction");
    exit(1);
  }

  if (3 != args) {
    printf("Parameter error\n\n\tSample: %s 127.0.0.1 50000\n", argv[0]);
    exit(1);
  }

  inet_aton(argv[1], &server_addr.sin_addr);
  server_addr.sin_family = PF_INET;
  server_addr.sin_port = htons(atoi(argv[2]));

  printf("Server IP: %s\n", inet_ntoa(server_addr.sin_addr));

  if (-1 == (sockfd = socket(PF_INET, SOCK_DGRAM, 0))) {
    perror("socket");
    exit(1);
  }

  int fcntl_flag;
  if (-1 == (fcntl_flag = fcntl(sockfd, F_GETFL, 0))) {
    perror("fcntl get flag");
    exit(1);
  }
  fcntl_flag |= O_NONBLOCK | O_ASYNC;
  if (-1 == fcntl(sockfd, F_SETFL, fcntl_flag)) {
    perror("fcntl set flag");
    exit(1);
  }
  if (-1 == fcntl(sockfd, F_SETOWN, getpid())) {
    perror("fcntl set own");
    exit(1);
  }

  int do_len;

  while (1) {
    printf("Message: ");
    memset(buf, 0, sizeof(buf));
    fflush(stdin);
    while (-1 == scanf("%s", buf)) {
      // ！被信号中断重新读取
      perror("scanf");
      printf("\nMessage: ");
    }

    if (0 == strncmp(buf, "quit", 4)) {
      printf("connect close\n");
      close(sockfd);
      return 0;
    }

    time(&td);
    tm = *localtime(&td);

    // 注意addr_len
    do_len = sendto(sockfd, buf, strlen(buf), 0,
                    (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (do_len == -1) {
      perror("sendto");
      exit(1);
    }

    // recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&server_addr,
    //          &server_addr_len);
    // printf("Recv successfully\n");
  }
  return 0;
}