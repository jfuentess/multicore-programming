// Compile: gcc -o timing_seq timing_seq.c lockfree_stack.c -fcilkplus -lcilkrts -lrt

#include <stdio.h>
#include <stdlib.h>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include "lockfree_stack.h"

#include <sys/time.h>
#include <time.h>


int main(int argc, char **argv)
{
  int i = 0;
  unsigned int operations = 0;
  
  if (argc != 2) {
    fprintf(stderr, "%s: <operations>\n", argv[0]);
    return 1;
  } 

  operations = atoi(argv[1]);
  struct timespec stime, etime;
  double t;
  double throughput;

  stack *s = stack_create();

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  for(int i = 0; i < operations; i++) {
    stack_push(s, i);
  }
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  throughput = (double)operations/t;
  
  printf("%s,%d,%lf,%u,%.lf\n", argv[0], __cilkrts_get_nworkers(), t, operations,
	 throughput);

  return EXIT_SUCCESS;
}
