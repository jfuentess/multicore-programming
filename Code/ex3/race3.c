// Compile: gcc -std=gnu99 -o race3 race3.c -fcilkplus -lcilkrts

#include <stdio.h>
#include <stdlib.h>

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

	int n = 100000;
	int x = 0;

	cilk_for(int i = 0; i < n; i++)
         __atomic_add_fetch(&x, 1, __ATOMIC_SEQ_CST); 

	printf("x: %d with %d threads (it should be %d)\n", x, num_threads, n);

	return EXIT_SUCCESS;
}
