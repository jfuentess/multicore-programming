/* Compile: gcc -std=gnu99 -o ps prefix_sum.c -fcilkplus -lcilkrts -lm -lrt */

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

#include <math.h>

#define num_threads __cilkrts_get_nworkers()

/* Sequential prefix block by block */
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
  
  /* for(uint i = 1; i < num_threads-1; i++) */
  /*   A[((i+1)*chk)-1] += A[i*chk-1]; */
  
  /* if(num_threads > 1) */
  /*   A[size-1] += A[(num_threads-1)*chk-1]; */
  
  /* cilk_for(uint i = 1; i < num_threads; i++) { */
  /*   uint ll = i*chk, ul = ll + chk - 1; */
  /*   if(i == num_threads-1) */
  /*     ul = size-1; */

  /*   uint acc = A[ll-1]; */
  /*   for(uint j = ll; j < ul; j++) { */
  /*     A[j] += acc; */
  /*   } */
  /* } */
}

/* Sequential prefix sum */
void prefix_sum_seq(uint *A, int n) {
  for(int i = 1; i < n; i++) {
    A[i] += A[i-1];
  }
}

/* Assuming n is a power of two */
/* To Do: Implement the version for any n */
void prefix_sum_iter(uint *A, unsigned int n) {
  if (n == 1){ 
    return;
  }   
  int itr;
  int levels = log2(n);

  // Up-sweep
  for (itr = 1; itr <= levels; itr++) {
    int desp = 1 << itr;
    
    cilk_for(unsigned int i = desp-1; i < n; i += desp){
      A[i] = A[i] + A[i - (1 << (itr - 1))];	
    }
  }

  // Down-sweep
  for (itr = levels-1; itr > 0; itr--) {
    int desp = 1 << itr;

    cilk_for(unsigned int i = desp-1; i < n-1; i += desp) {
      int idx = i + (1 << (itr-1));
      A[idx] += A[i];	
    }
  }
}

void upsweep(uint *A, uint ll, uint ul) {
  if(ul - ll == 1) {
    A[ul] += A[ll];
    return;
  }
  uint k = pow(2,((int)log2(ul-ll+1)-1));

  cilk_spawn upsweep(A, ll, ll+k-1);
  upsweep(A, ll+k, ul);
  cilk_sync;
  A[ul] += A[ul-k];
}

void downsweep(uint *A, uint ll, uint ul, int n) {
  if(ul - ll == 0) {
    return;
  }

  uint k = pow(2,((int)log2(ul-ll+1)-1));

  if(ul+k < n)
    A[ul+k] += A[ul];
  
  cilk_spawn downsweep(A, ll, ll+k-1, n);
  downsweep(A, ll+k, ul, n);
  // Implicit cilk_sync
}

/* Assuming n is a power of two */
/* To Do: Implement the version for any n */
void prefix_sum_tree(uint *A, int n) {
  upsweep(A, 0, n-1);
  downsweep(A, 0, n-1, n);
}

int main(int argc, char* argv[]) {
  
  struct timespec stime, etime;
  double t;
  
  int n = atoi(argv[1]);
  uint *A = malloc(n*sizeof(uint));
	
  for(int i=0; i<n; i++) {
    A[i] = 1;
  }

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

#if defined(BLOCK)
  prefix_sum_block(A, n);
#elif defined(TREE)
  prefix_sum_tree(A, n);
#elif defined(ITER)
  prefix_sum_iter(A, n);
#elif defined(SEQ)
  prefix_sum_seq(A, n);
#endif
    
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  printf("%d,%s,%lf,%u\n", __cilkrts_get_nworkers(), argv[0], t, n);
 
  return EXIT_SUCCESS;
}
