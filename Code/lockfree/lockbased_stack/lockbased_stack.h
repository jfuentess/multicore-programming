#ifndef _LBS_H
#define _LBS_H

#include <pthread.h> //pthread library	

typedef struct _element_t element;
typedef struct _stack_t stack;

struct _element_t{
  unsigned int val;
  element *next;
};

struct _stack_t {
  pthread_mutex_t lock;
  element *top;
};

stack * stack_create();
unsigned int stack_pop(stack *);
unsigned int stack_pop_ts(stack *);
void stack_push(stack *, unsigned int);
void stack_push_ts(stack *, unsigned int);
void print_stack(stack *);
unsigned int stack_size(stack *);

#endif /* _LBS_H */
