#include <stdlib.h>
#include <stdio.h>

#include "defs.h"

/*
 * Stack implementation. The implementation is based on the code in Chapter 1.1
 * of "Advanced Data Structures" by Peter Brass.
 *
 * This implementation is good in terms of performance, but not in terms of
 * memory usage. However, in this project we prefer a good performance than a
 * memory usage.
 */

Stack *stack_create(uint size) {
  Stack *s;
  s = (Stack *) malloc(sizeof(Stack));
  s->base = (item_t *) malloc(size * sizeof(item_t));
  s->size = size;
  s->top = 0;

  return s;
}

int stack_empty(Stack *s) {
  return (s->top == 0);
}

int stack_full(Stack *s) {
  return (s->top == s->size);
}

int stack_push(Stack *s, item_t x) {
  if(!stack_full(s)) {
    s->base[s->top] = x;
    s->top += 1;

    if(s->top > s->size)
      printf("push - top: %d\n", s->top);

    return 1;
  }
  else {
    fprintf(stderr, "Error in function stack_push: The stack is full\n");
    exit(EXIT_FAILURE);
  }
}

item_t stack_pop(Stack *s) {
  if(!stack_empty(s)) {
    s->top -= 1;
    if(s->top > s->size)
      printf("pop - top: %d\n", s->top);
    return s->base[s->top];
  }
  else {
    fprintf(stderr, "Error in function stack_pop: The stack is empty\n");
    exit(EXIT_FAILURE);
  }
}

item_t stack_top(Stack *s) {
  if(!stack_empty(s))
    return s->base[s->top];
  else {
    fprintf(stderr, "Error in function stack_top: The stack is empty\n");
    exit(EXIT_FAILURE);
  }
}

void stack_free(Stack *s) {
  free(s->base);
  free(s);
}

int tree_cycle(Tree* t) {
  Stack *s = stack_create(t->n); 
  char* visited = calloc(t->n, sizeof(char));
  char* edges = calloc(2*(t->n-1), sizeof(char));
  int par, curr = -1;
  unsigned int edge;
  int first = 1;
  unsigned int i = 0;

  while(!stack_empty(s) || first) {
    if(first) { // Root
      par = -1;
      curr = 0;
      first = 0;
    }
    else {
      edge = stack_pop(s);
      par = t->E[edge].src; // parent
      curr = t->E[edge].tgt; // current
    }
    visited[curr] = 1;
    for(i = t->N[curr].first; i <= t->N[curr].last; i++) {
      edges[i] = 1;
      if(!visited[t->E[i].tgt])
	stack_push(s, i);      
      else if(par != t->E[i].tgt && par != -1) {
	printf("\tThe tree has cycles\n");
	return 1;
      }
    }
  }

  for(i = 0; i < t->n; i++)
    if(visited[i] == 0) { // There are unvisited nodes
      printf("\tNo connected tree (unvisited nodes)\n");
      return 1;
    }

  for(i = 0; i < 2*(t->n-1); i++)
    if(edges[i] == 0) { // There are unvisited edges
      printf("\tNo connected graph (unvisited edges)\n");
      return 1;
    }
  
  return 0;
}
