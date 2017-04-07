// Compilar: gcc -std=gnu99 -o fibonacci fibonacci.c -fcilkplus -lcilkrts

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

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

#define num_threads __cilkrts_get_nworkers();

int fib(int n)
{
    if (n < 2)
        return n;
    int x = cilk_spawn fib(n-1);
    int y = fib(n-2);
    cilk_sync;
    return x + y;
}

int main(int argc, char* argv[]) {

	struct timespec stime, etime;
	double t;

	int n = atoi(argv[1]);

	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
		fprintf(stderr, "clock_gettime failed");
		exit(-1);
	}

	int result = fib(n);
	
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
		fprintf(stderr, "clock_gettime failed");
		exit(-1);
	}

	printf("Fibonnacci(%d): %d\n", n, result);

	t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

	printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);

	return EXIT_SUCCESS;
}
