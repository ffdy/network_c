#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <unistd.h>

int sockfd, newfd;
struct sockaddr_in client_addr;
socklen_t client_addr_len = sizeof(client_addr); // !非常重要，不然获取不到client_addr
char buf[1024];

void sigio_handler(int signo) {
  sleep(5);
  bzero(buf, sizeof buf);
  while (-1 !=
      recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_len)) {
    printf("Message from %s: %s\n", inet_ntoa(client_addr.sin_addr), buf);
    char send_buf[] = "ACK";
    if (-1 == sendto(sockfd, send_buf, strlen(send_buf), 0, (struct sockaddr *)&client_addr,
                     sizeof(client_addr))) {
      perror("sendto");
    }
    printf("Send success\n");
  }

  perror("recvfrom");
  return;
}

int main(int args, char *argv[]) {
  struct sigaction action;
  bzero(&action, sizeof(struct sigaction));
  action.sa_handler = sigio_handler;
  if (-1 == sigaction(SIGIO, &action, NULL)) {
    perror("sigaction");
    exit(1);
  }

  if (-1 == (sockfd = socket(PF_INET, SOCK_DGRAM, 0))) {
    perror("socket");
    exit(1);
  }
  int fcntl_flag;
  if (-1 == (fcntl_flag = fcntl(sockfd, F_GETFL, 0))) {
    perror("fcntl get flag");
    exit(1);
  }
  fcntl_flag |= O_ASYNC | O_NONBLOCK;
  if (-1 == fcntl(sockfd, F_SETFL, fcntl_flag)) {
    perror("fcntl set flag");
    exit(1);
  }
  if (-1 == fcntl(sockfd, F_SETOWN, getpid())) {
    perror("fcntl set own");
    exit(1);
  }

  struct sockaddr_in server_addr;
  socklen_t server_addr_len;
  server_addr.sin_family = PF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(atoi(argv[1]));

  if (-1 ==
      bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
    perror("bind");
    exit(1);
  }

  while (1) {
    
  }

  return 0;
}