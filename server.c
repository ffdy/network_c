#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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

  sockfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (-1 == sockfd) {
    perror("socket");
    exit(1);
  }

  if (-1 ==
      bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
    perror("bind");
    exit(1);
  }

  int do_len;
  printf("Start to be connected\n");

  while (1) {
    // do_len = read(newfd, buf, sizeof(buf));
    do_len = recvfrom(sockfd, buf, sizeof(buf), 0,
                      (struct sockaddr *)&client_addr, &client_addr_len);
    printf("Message from %s : %s\n", inet_ntoa(client_addr.sin_addr), buf);

    sendto(sockfd, "ACK", 3, 0, (struct sockaddr *)&client_addr,
           client_addr_len);
  }

  return 0;
}