#include <stdlib.h>
#include <stdio.h>

#include "lockfree_stack.h"

stack * stack_create()
{
  stack *s = (stack *)malloc(sizeof(stack));
  s->top = NULL;
  return s;
}

// Thread-safe version
void stack_push_ts(stack *s, unsigned int data) {
  element *top, *new;

  new = (element *)malloc(sizeof(element));
  new->val = data;
  
  do {
    top = s->top;

    new->next  = top;
    
    if(__atomic_compare_exchange_n(&(s->top), &top, new, 0, __ATOMIC_SEQ_CST,
    				   __ATOMIC_SEQ_CST )) {
      return;
    }
  } while(1);
}

// Non Thread-safe version
void stack_push(stack *s, unsigned int data) {
  element *new;
  
  new = (element *)malloc(sizeof(element));
  new->val = data;
  new->next  = s->top;    
  s->top = new;
}

// Non Thread-safe version
unsigned int stack_pop(stack *s) {
  if(s->top != NULL) {
    element *tmp = s->top;
    unsigned int val = tmp->val;
    s->top = tmp->next;
    free(tmp);
    return val;
  }
  return -1;
}

// Thread-safe version
unsigned int stack_pop_ts(stack *s) {
  if(s->top != NULL) {
    element *tmp, *new;
    unsigned int val;
    
    do {
      tmp = s->top;
      val = tmp->val;
      new = tmp->next;
      
      if(__atomic_compare_exchange_n(&(s->top), &tmp, new, 0, __ATOMIC_SEQ_CST,
				     __ATOMIC_SEQ_CST )) {
	free(tmp);
	return val;
      }
    } while(1);
  }
}

void print_stack(stack *s)
{
  element *e = s->top;

  printf("Stack: ");
  while(e != NULL) {
    printf("%u ", e->val);
    e = e->next;
  }
  printf("\n");
}

unsigned int stack_size(stack *s)
{
  element *e = s->top;
  unsigned int size = 0;
  while(e != NULL) {
    size++;
    e = e->next;
  }
  return size;
}

