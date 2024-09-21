#ifndef lcthw_list_h
#define lcthw_list_h

#include <stdlib.h>

struct ListNode;

typedef struct ListNode {
  struct ListNode *next;
  struct ListNode *prev;
  void *value;
} ListNode;

typedef struct List {
  int count;
  ListNode *first;
  ListNode *last;
} List;

List *List_create();
/** free all the nodes and the list, but not the value fields */
void List_destroy(List *list);
/** free the value field of all nodes */
void List_clear(List *list);
void List_clear_destroy(List *list);

#define List_count(A) (assert((A) != NULL), (A)->count)
#define List_first(A)                                                          \
  (assert((A) != NULL), (A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A)                                                           \
  (assert((A) != NULL), (A)->last != NULL ? (A)->last->value : NULL)

/** push back */
void List_push(List *list, void *value);
/** pop back */
void *List_pop(List *list);

/** push front */
void List_unshift(List *list, void *value);
/** pop front */
void *List_shift(List *list);

void *List_remove(List *list, ListNode *node);

#define LIST_FOREACH(L, S, M, V)                                               \
  ListNode *_node = NULL;                                                      \
  ListNode *V = NULL;                                                          \
  for (V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif // lcthw_list_h
