#include <assert.h>
#include <dbg.h>
#include <lcthw/list.h>
#include <stdlib.h>

List *List_create() { return calloc(1, sizeof(List)); }

void List_destroy(List *list) {
  assert(list != NULL);
  LIST_FOREACH(list, first, next, curr) {
    if (curr->prev) {
      free(curr->prev);
    }
  }

  free(list->last);
  free(list);
}

void List_clear(List *list) {
  assert(list != NULL);
  LIST_FOREACH(list, first, next, curr) { free(curr->value); }
}

void List_clear_destroy(List *list) {
  assert(list != NULL);
  LIST_FOREACH(list, first, next, curr) {
    free(curr->value);
    if (curr->prev) {
      free(curr->prev);
    }
  }
  free(list->last);
  free(list);
}

void List_push(List *list, void *value) {
  assert(list != NULL);
  ListNode *node = calloc(1, sizeof(ListNode));
  check_mem(node);

  node->value = value;
  if (list->last == NULL) {
    list->first = node;
    list->last = node;
  } else {
    list->last->next = node;
    node->prev = list->last;
    list->last = node;
  }
  list->count++;
error:
  return;
}

void *List_pop(List *list) {
  assert(list != NULL);
  ListNode *node = list->last;
  return node != NULL ? List_remove(list, node) : NULL;
}

void List_unshift(List *list, void *value) {
  assert(list != NULL);
  ListNode *node = calloc(1, sizeof(ListNode));
  check_mem(node);

  node->value = value;
  if (list->first == NULL) {
    list->first = node;
    list->last = node;
  } else {
    node->next = list->first;
    list->first->prev = node;
    list->first = node;
  }
  list->count++;

error:
  return;
}

void *List_shift(List *list) {
  assert(list != NULL);
  ListNode *node = list->first;
  return node != NULL ? List_remove(list, node) : NULL;
}

void *List_remove(List *list, ListNode *node) {
  assert(list != NULL);
  void *result = NULL;
  check(list->first && list->last, "List is empty");
  check(node, "node can't be NULL");

  if (node == list->first && node == list->last) {
    list->first = NULL;
    list->last = NULL;
  } else if (node == list->first) {
    list->first = node->next;
    check(list->first != NULL,
          "Invalid list, somehow got a first that is NULL.");
    list->first->prev = NULL;
  } else if (node == list->last) {
    list->last = node->prev;
    check(list->last != NULL, "Invalid list, somehow got a next that is NULL.");
    list->last->next = NULL;
  } else {
    ListNode *after = node->next;
    ListNode *before = node->prev;
    after->prev = before;
    before->next = after;
  }
  list->count--;
  result = node->value;
  free(node);

error:
  return result;
}
