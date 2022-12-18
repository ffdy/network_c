#ifndef list__
#define list__
struct list_t {
  void *elem;
  struct list_t *prev;
  struct list_t *next;
};
#endif

void list_init(struct list_t *p_list);
void *list_head(struct list_t list);
void *list_tail(struct list_t list);
void *list_add_head(void *elem, struct list_t *list);
void *list_add_tail(void *elem, struct list_t *list);
void list_del(struct list_t *elem);
int list_empty(struct list_t *list);