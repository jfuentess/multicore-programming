// Compile: gcc -std=gnu99 -o race1 race1.c -fcilkplus -lcilkrts

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

void add(int *x) {
	*x = *x + 1;
}

int main(int argc, char* argv[]) {

	int n = 10000000;
	int x = 0;

	for(int i = 0; i < n; i++)
		cilk_spawn add(&x);
	cilk_sync;

	printf("x: %d with %d threads (it should be %d)\n", x, num_threads, n);

	return EXIT_SUCCESS;
}
