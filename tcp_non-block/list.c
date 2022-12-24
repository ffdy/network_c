#include <malloc.h>
#include <stdlib.h>
#include "list.h"

void list_init(struct list_t *p_list) {
  p_list->next = p_list;
  p_list->prev = p_list;
}
void *list_head(struct list_t list) { return list.next->elem; }

void *list_tail(struct list_t list) { return list.prev->elem; }

void *list_add_head(void *elem, struct list_t *list) {
  struct list_t *new_elem_node = (struct list_t *)malloc(sizeof(struct list_t));
  new_elem_node->elem = elem;
  new_elem_node->prev = list;
  new_elem_node->next = list->next;
  list->next->prev = new_elem_node;
  list->next = new_elem_node;
  return new_elem_node;
}

void *list_add_tail(void *elem, struct list_t *list) {
  struct list_t *new_elem_node = (struct list_t *)malloc(sizeof(struct list_t));
  new_elem_node->elem = elem;
  new_elem_node->next = list;
  new_elem_node->prev = list->prev;
  list->prev->next = new_elem_node;
  list->prev = new_elem_node;
  return new_elem_node;
}

void list_del(struct list_t *elem) {
  if (elem == NULL)
    return;
  elem->prev->next = elem->next;
  elem->next->prev = elem->prev;
  free(elem);
}

int list_empty(struct list_t *list) {
  if (list->next == list)
    return 1;
  return 0;
}