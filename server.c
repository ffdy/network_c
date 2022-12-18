#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>

#include "list.h"

struct sockfd_opt {
  int fd;
  int (*do_task)(struct sockfd_opt *p_so);
  struct list_t *list_node;
};

#define MAX(sockfd, nfds) (sockfd > nfds ? sockfd : nfds)

int sockfd, newfd, so_reuseaddr = 1;
struct sockaddr_in server_addr, client_addr;
socklen_t server_addr_len, client_addr_len;

char buf[1024];

int fd_list_len = 0;
struct list_t fd_list;
int nfds = -1;
fd_set readset, old_readset;

int read_from_client(struct sockfd_opt *p_so) {
  bzero(buf, sizeof buf);
  int do_len = read(p_so->fd, buf, sizeof(buf));
  if (do_len == 0) {
    // 连接关闭
    printf("connect from %s close\n", inet_ntoa(client_addr.sin_addr));
    close(p_so->fd);
    FD_CLR(p_so->fd, &old_readset);

    list_del(p_so->list_node);
    free(p_so);

    return 0;
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
    return 1;
  }
  printf("Connect from %s\n", inet_ntoa(client_addr.sin_addr));

  FD_SET(p->fd, &old_readset);
  nfds = MAX(nfds, p->fd);
  p->do_task = read_from_client;
  p->list_node = list_add_tail(p, &fd_list);

  return 0;
}

int do_tast(int num) {
  printf("%d\n", num);
  return 0;
}

struct list_test {
  int a;
  int (*do_task)(int num);
  struct list_t *list_ptr;
};

void task() {
  struct list_t test_list;
  list_init(&test_list);
  for (int i = 0; i < 10; i++) {
    struct list_test *list_testa =
        (struct list_test *)malloc(sizeof(struct list_test));
    list_testa->a = i;
    list_testa->do_task = do_tast;
    list_testa->list_ptr = list_add_tail(list_testa, &test_list);
  }

  for (struct list_t *p = test_list.next; p != &test_list; p = p->next) {
    ((struct list_test *)p->elem)->do_task(((struct list_test *)p->elem)->a);
    if (((struct list_test *)p->elem)->a == 5) {
      list_del(p);
    }
  }

  for (struct list_t *p = test_list.next; p != &test_list; p = p->next) {
    ((struct list_test *)p->elem)->do_task(((struct list_test *)p->elem)->a);
  }

}

int main(int args, char *argv[]) {

  // task();

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

  if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr,
                       sizeof(so_reuseaddr))) {
    perror("setsockopt");
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
  
  FD_ZERO(&old_readset);
  FD_SET(sockfd, &old_readset);
  nfds = MAX(sockfd, nfds);
  list_init(&fd_list);

  struct sockfd_opt sockfd_opt;
  sockfd_opt.fd = sockfd;
  sockfd_opt.do_task = create_newfd;
  sockfd_opt.list_node = list_add_tail(&sockfd_opt, &fd_list);

  while (1) {
    readset = old_readset;
    select(nfds + 1, &readset, NULL, NULL, NULL);

    for (struct list_t *p = fd_list.next; p != &fd_list; p = p->next) {
      if (FD_ISSET(((struct sockfd_opt *)p->elem)->fd, &readset)) {
        ((struct sockfd_opt *)p->elem)->do_task((struct sockfd_opt *)p->elem);
      }
    }
  }

  return 0;
}