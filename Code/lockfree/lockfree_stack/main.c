// Compile: gcc -o lfs main.c lockfree_stack.c -fcilkplus -lcilkrts

#include <stdio.h>
#include <stdlib.h>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include "lockfree_stack.h"


int main(int argc, char **argv)
{
  int i = 0;
  unsigned int iterations = 0;
  
  if (argc != 2) {
    fprintf(stderr, "%s: <iterations>\n", argv[0]);
    return 1;
  } 

  iterations = atoi(argv[1]);

  stack *s1 = stack_create();
  stack *s2 = stack_create();

  printf("Using %d threads\n", __cilkrts_get_nworkers());

  printf("Thread-safe version\n");
  cilk_for(int i=0; i<iterations; i++) {
    stack_push_ts(s2, i);
  }
  printf("\tAfter %u insertions: %u\n", iterations, stack_size(s2));
  cilk_for(int i=0; i<iterations; i++) {
    stack_pop_ts(s2);
  }
  printf("\tAfter %u deletions: %u\n", iterations, stack_size(s2));

  printf("Non thread-safe version\n");
  cilk_for(int i=0; i<iterations; i++) {
    stack_push(s1, i);
  }
  printf("\tAfter %u insertions: %u\n", iterations, stack_size(s1));
  cilk_for(int i=0; i<iterations; i++) {
    stack_pop(s1);
  }
  printf("\tAfter %u deletions: %u\n", iterations, stack_size(s1));

  return EXIT_SUCCESS;
}
