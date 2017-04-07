// Compilar: gcc -std=gnu99 -o exfor example_for.c -fcilkplus -lcilkrts

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

  int n = 3000;
  cilk_for(int i = 0; i < n; i++) {
    printf("%d %d\n", __cilkrts_get_worker_number(), i);
  }
  
  return EXIT_SUCCESS;
}
