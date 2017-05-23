// Compilar: g++ -std=c++11 -o ss sequential_sort.cpp -lrt

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

#include <algorithm>

/*
 * STL functions used in this code:
 *
 * + std::stable_sort(first, last): Sorts the elements in the range [first,last)
 *   into ascending order, like sort, but stable_sort preserves the relative
 *   order of the elements with equivalent values. 
 *
 */

int *generate_array(int n) {
  int *x = (int *)malloc(n*sizeof(int));
  
  for (int i = 0; i < n; i++)
    x[i] = rand();

  return x;
}

int main(int argc, char* argv[]) {

  if(argc < 2) {
    fprintf(stderr, "Usage: %s <number of elements>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int n = atoi(argv[1]);
  int *x = generate_array(n);
  
  struct timespec stime, etime;
  double t;
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  std::stable_sort( x, x+n );

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("threads: 1, elapsed time: %lf\n", t);
  
  return EXIT_SUCCESS;
}
