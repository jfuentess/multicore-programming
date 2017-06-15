#include <stdlib.h>
#include <stdio.h>

#include "lockbased_stack.h"

stack * stack_create()
{
  stack *s = (stack *)malloc(sizeof(stack));
  s->top = NULL;
  pthread_mutex_init(&s->lock,NULL); //initialize the lock
  return s;
}

// Thread-safe version
void stack_push_ts(stack *s, unsigned int data) {
  element *new;  
  new = (element *)malloc(sizeof(element));
  new->val = data;

  pthread_mutex_lock(&s->lock); // Acquire the lock

  new->next  = s->top;    
  s->top = new;

  pthread_mutex_unlock(&s->lock); // Release the lock

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
  pthread_mutex_lock(&s->lock); // Acquire the lock

  if(s->top != NULL) {
    element *tmp = s->top;
    unsigned int val = tmp->val;
    s->top = tmp->next;
    free(tmp);
    
    pthread_mutex_unlock(&s->lock); // Release the lock

    return val;
  }
  
  pthread_mutex_unlock(&s->lock); // Release the lock
  return -1;
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

