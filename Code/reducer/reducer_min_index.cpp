/* Compile: g++ -o min_index reducer_min_index.cpp -fcilkplus -lcilkrts -lrt */
using namespace std;

#include <stdio.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_min.h>

#include <sys/time.h>
#include <time.h>

void reducer_min_test(uint* A, int n)
{
  /* Reducer to compute the index of the minimum value of an array */
  cilk::reducer< cilk::op_min_index<uint, uint> > best;
  
  cilk_for(unsigned i = 0; i < n; ++i) {
    best->calc_min(i, A[i]);
  }
  
  printf("Minimum index = %d, value = %u\n",
  	best->get_index_reference(),
  	best->get_reference());
}

int main(int argc, char* argv[])
{
  if(argc != 2) {
    fprintf(stderr, "Usage: %s <number of elements>\n", argv[0]);
    return EXIT_FAILURE;
  }

  struct timespec stime, etime;
  double t;

  int n = atoi(argv[1]);
  uint *A = new uint[n];

  for(int i=0; i<n; i++)
    A[i] = i+1;

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  reducer_min_test(A, n);

  	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
		fprintf(stderr, "clock_gettime failed");
		exit(-1);
	}

	t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

	printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);

  return 0;
}
