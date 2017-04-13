// Compilar: gcc -std=gnu99 -o scaled scaled.c -fcilkplus -lcilkrts

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>
#include <math.h>

#ifdef NOPARALLEL
#define __cilkrts_get_nworkers() 1
#define cilk_for for
#define cilk_spawn 
#define cilk_sync 
#else
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/common.h>
#endif

#define num_threads __cilkrts_get_nworkers()

void scaled_vector_add(int* x, int* y, int a, int n) {
  //#pragma cilk grainsize = 1
  cilk_for(int i = 0; i < n; i++) {
    y[i] = a*x[i] + y[i];
  }
}


int main(int argc, char* argv[]) {

  struct timespec stime, etime;
  double t;

  int n = atoi(argv[1]);
  int a = atoi(argv[2]);
  
  int *x = (int *)malloc(n*sizeof(int));
  int *y = (int *)malloc(n*sizeof(int));

  for(int i = 0; i < n; i++) {
    x[i] = 1;
    y[i] = 1;
  }

  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  scaled_vector_add(x, y, a, n);

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);
  
  return EXIT_SUCCESS;
}
