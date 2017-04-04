// Compile: gcc -std=gnu99 -o falsesharing2 falsesharing2.c -fcilkplus -lcilkrts -lm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

#define num_threads __cilkrts_get_nworkers()

int main(int argc, char* argv[]) {

	int n = atoi(argv[1]);
	n = (n/num_threads)*num_threads;
	int c = atoi(argv[2]);
	int* random_list = malloc(n*sizeof(int));
	time_t t;

	srand((unsigned) time(&t));

	for(int i = 0; i < n; i++)
		random_list[i] = rand() % 10;

	struct timespec stime, etime;
	double et;

	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
		fprintf(stderr, "clock_gettime failed");
		exit(-1);
	}

	cilk_for(int i = 0; i < n; i++)
		random_list[i] *= c;

	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
		fprintf(stderr, "clock_gettime failed");
		exit(-1);
	}

	et = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

	printf("%d,%lf,%d,%s\n", num_threads, et, n, argv[0]);

	return EXIT_SUCCESS;
}
