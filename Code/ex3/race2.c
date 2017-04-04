// Compile: gcc -std=gnu99 -o race2 race2.c -fcilkplus -lcilkrts

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

	int n = 1000;
	cilk_for(int i = 0; i < num_threads; i++)
		for(int j = 0; j < n; j++)
			printf("%d", i);

	printf("\n");

	return EXIT_SUCCESS;
}
