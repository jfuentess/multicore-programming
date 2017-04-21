// Compile: gcc -std=gnu99 -o template template.c -fcilkplus -lcilkrts

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

#define num_threads __cilkrts_get_nworkers()

int main(int argc, char* argv[]) {

	struct timespec stime, etime;
	double t;

	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
		fprintf(stderr, "clock_gettime failed");
		exit(-1);
	}

	/* YOUR PARALLEL CODE*/
	
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
		fprintf(stderr, "clock_gettime failed");
		exit(-1);
	}

	t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

	printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);

	return EXIT_SUCCESS;
}
