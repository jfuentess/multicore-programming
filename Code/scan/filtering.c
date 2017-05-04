// Compile: gcc -std=gnu99 -o filtering filtering.c util.c defs.c -fcilkplus -lcilkrts -lrt

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

int filter(int v) {
  return (v%2 == 0);
}

void prefix_sum_block(uint* A, uint size) {
  uint chk = size/num_threads;
  cilk_for(uint i = 0; i < num_threads; i++) {
    uint  ll = i*chk, ul = ll + chk;
    if(i == num_threads-1)
      ul = size;

    uint acc = 0;
    for(uint j = ll; j < ul; j++) {
      A[j] += acc;
      acc = A[j];
    }
  }
  
  for(uint i = 1; i < num_threads-1; i++)
    A[((i+1)*chk)-1] += A[i*chk-1];
  
  if(num_threads > 1)
    A[size-1] += A[(num_threads-1)*chk-1];
  
  cilk_for(uint i = 1; i < num_threads; i++) {
    uint ll = i*chk, ul = ll + chk - 1;
    if(i == num_threads-1)
      ul = size-1;

    uint acc = A[ll-1];
    for(uint j = ll; j < ul; j++) {
      A[j] += acc;
    }
  }
}

int* filtering_seq(int *A, int n, int *num_success) {

  int local = 0;
  for(int i=0; i<n; i++)
    if(filter(A[i])) {
      local++;
    }

  *num_success = local;
  int *output = malloc(local*sizeof(int));
  int idx = 0;
  
  for(int i=0; i<n; i++)
    if(filter(A[i])) {
      output[idx] = A[i];
      idx++;
    }

  return output;
}

int* filtering(int *A, int *B, int size) {
  int num_success = 0;
  int *partial = malloc(num_threads*sizeof(int));
  uint chk = size/num_threads;
  // Aplicar el filtro
  cilk_for(uint i = 0; i < num_threads; i++) {
    uint  ll = i*chk, ul = ll + chk;
    if(i == num_threads-1)
      ul = size;

    int counter = 0;
    for(uint j = ll; j < ul; j++) {
      B[j] = filter(A[j]);
      counter += B[j];
    }

    partial[i] = counter;
  }

  // Calcular la cantidad total de elementos que pasaron el filtro
  for(int i=0; i<num_threads; i++)
    num_success += partial[i];

  prefix_sum_block(B, size);
  int *output = malloc(num_success*sizeof(int));

  // Llenar el arreglo final
  cilk_for(uint i = 0; i < num_threads; i++) {
    uint  ll = i*chk, ul = ll + chk;
    if(ll == 0)
      ll++;
    if(i == num_threads-1)
      ul = size;

    for(uint j = ll; j < ul; j++) {
      if(B[j] > B[j-1]) {
	int x = B[j];
	output[x] = A[j];
      }
    }
  }

  if(B[0])
    output[0] = A[0];
  
  return output;
}

int main(int argc, char* argv[]) {

  struct timespec stime, etime;
  double t;

  int n = atoi(argv[1]);
  int *A = malloc(n*sizeof(n));
  int *B = calloc(n, sizeof(n));
  
  for(int i=0; i<n; i++)
    A[i] = i;
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  int num_success = 0;
  //  int* output_par = filtering(A, B, n);
  int* output_seq = filtering_seq(A, n, &num_success);

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);

  /* for(int i=0; i<num_success; i++) { */
  /*   if(output_par[i] != output_seq[i]) { */
  /*     printf("Diff at position %d (%d != %d)\n", i, output_par[i], output_seq[i]); */
  /*   } */
  /* } */
  return EXIT_SUCCESS;
}
