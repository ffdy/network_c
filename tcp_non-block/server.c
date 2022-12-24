#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "list.h"

struct sockfd_opt {
  int fd;
  int (*do_task)(struct sockfd_opt *p_so);
  struct list_t *list_node;
};

int sockfd, newfd, so_reuseaddr = 1;
struct sockaddr_in server_addr, client_addr;
socklen_t server_addr_len, client_addr_len;

char buf[1024];
struct list_t fd_list;

int read_from_client(struct sockfd_opt *p_so) {
  bzero(buf, sizeof buf);
  int do_len = read(p_so->fd, buf, sizeof(buf));
  if (do_len == 0) {
    // 连接关闭
    printf("connect from %s close\n", inet_ntoa(client_addr.sin_addr));
    close(p_so->fd);

    list_del(p_so->list_node);
    free(p_so);

    return 0;
  } else if (do_len == -1) {
    perror("read");
    return 1;
  }
  printf("Message from %s : %s\n", inet_ntoa(client_addr.sin_addr), buf);

  write(p_so->fd, "ACK", 3);
  return 0;
}

int create_newfd(struct sockfd_opt *p_so) {
  struct sockfd_opt *p = (struct sockfd_opt *)malloc(sizeof(struct sockfd_opt));
  if (-1 == (p->fd = accept(p_so->fd, (struct sockaddr *)&client_addr,
                            &client_addr_len))) {
    perror("accept");
    if (errno == EAGAIN)
      return 0;
    exit(1);
  }
  printf("Connect from %s\n", inet_ntoa(client_addr.sin_addr));

  p->do_task = read_from_client;
  p->list_node = list_add_tail(p, &fd_list);

  int fcntl_flags;
  if (-1 == (fcntl_flags = fcntl(p->fd, F_GETFL, 0))) {
    perror("fcntl get flags");
    exit(1);
  }

  fcntl_flags |= O_NONBLOCK;
  if (-1 == fcntl(p->fd, F_SETFL, fcntl_flags)) {
    perror("fcntl set flags");
    exit(1);
  }

  return 0;
}

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

  // socket地址复用
  if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr,
                       sizeof(so_reuseaddr))) {
    perror("setsockopt");
    exit(1);
  }

  // 获取当前工作模式
  int fcntl_flags;
  if (-1 == (fcntl_flags = fcntl(sockfd, F_GETFL, 0))) {
    perror("fcntl getfl");
    exit(1);
  }

  // 添加设置非组设工作模式
  fcntl_flags |= O_NONBLOCK;
  if (-1 == fcntl(sockfd, F_SETFL, fcntl_flags)) {
    perror("fcntl setfl");
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
  list_init(&fd_list);

  struct sockfd_opt sockfd_opt;
  sockfd_opt.fd = sockfd;
  sockfd_opt.do_task = create_newfd;
  sockfd_opt.list_node = list_add_tail(&sockfd_opt, &fd_list);

  while (1) {
    sleep(1);
    for (struct list_t *p = fd_list.next; p != &fd_list; p = p->next) {
      ((struct sockfd_opt *)p->elem)->do_task((struct sockfd_opt *)p->elem);
    }
  }

  return 0;
}