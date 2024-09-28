#ifndef lcthw_stack_h_
#define lcthw_stack_h_
#include <lcthw/list.h>

typedef List Stack;

Stack *Stack_create() { return List_create(); }
void Stack_destroy(Stack *stack) { return List_clear_destroy(stack); }
void Stack_push(Stack *stack, void *value) { return List_push(stack, value); }
void *Stack_pop(Stack *stack) { return List_pop(stack); }
#define Stack_count(A) (List_count(A))
#define STACK_FOREACH(S, V) LIST_FOREACH(S, first, next, V)
#define Stack_peek(A) List_last(A)

#endif // lcthw_stack_h_
