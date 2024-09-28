#ifndef lcthw_queue_h_
#define lcthw_queue_h_
#include <lcthw/list.h>

typedef List Queue;

Queue *Queue_create() { return List_create(); }
void Queue_destroy(Queue *queue) { return List_clear_destroy(queue); }
void Queue_send(Queue *queue, void *value) {
  return List_unshift(queue, value);
}
void *Queue_recv(Queue *queue) { return List_pop(queue); }
#define Queue_count(A) (List_count(A))
#define QUEUE_FOREACH(S, V) LIST_FOREACH(S, first, next, V)
#define Queue_peek(A) List_last(A)

#endif // lcthw_queue_h_
