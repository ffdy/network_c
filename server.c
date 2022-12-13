#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

pid_t sub_pid;
struct sigaction sigchld_action;

int sockfd, newfd;
struct sockaddr_in server_addr, client_addr;
socklen_t server_addr_len, client_addr_len;

char buf[1024];

static void sigchld_handler(int signo) {

  pid_t pid;
  int status;

  do {
    if (-1 == (pid = waitpid(-1, &status, WNOHANG | WUNTRACED))) {
      perror("waitpid");
    }
    if (pid > 0) {
      printf("Child process exited PID%d, Exit Code: %d\n", pid, status);
    } else if (0 == pid) {
      printf("non-blocking\n");
    }
  } while (pid > 0);
}

int main(int args, char *argv[]) {

  sigchld_action.sa_flags = SA_RESTART;
  sigchld_action.sa_handler = sigchld_handler;
  if (-1 == (sigaction(SIGCHLD, &sigchld_action, NULL))) {
    perror("sigaction");
    exit(1);
  }

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
          // 连接关闭
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
      // 很重要，父子进程都打开了这个套接字
      close(newfd);
    }
  }

  return 0;
}