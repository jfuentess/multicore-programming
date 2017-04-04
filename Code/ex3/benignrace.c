// Compile: gcc -std=gnu99 -o benignrace benignrace.c -fcilkplus -lcilkrts

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

	int n = 30;
	int alphabet = 10;
	int* sequence = malloc(n*sizeof(int));
	int* check_list = calloc(alphabet, sizeof(int));
	time_t t;

	srand((unsigned) time(&t));

	for(int i = 0; i < n; i++)
		sequence[i] = rand() % alphabet;
	
	cilk_for(int i = 0; i < n; i++)
		check_list[sequence[i]] = 1;

	printf("check list:\n");
	for(int i = 0; i < alphabet; i++)
		if(check_list[i])
			printf("\t %d IS in the sequence\n", i);
		else
			printf("\t %d IS NOT in the sequence\n", i);
	printf("\n");

	return EXIT_SUCCESS;
}
