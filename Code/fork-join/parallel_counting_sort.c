// Compilar: gcc -std=gnu99 -o pcs parallel_counting_sort.c -fcilkplus -lcilkrts -lrt

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

// Exclusive parallel prefix sum
// Returns the total sum
uint parallel_prefix_sum(uint* A, uint size) {
  uint chk = size/num_threads;
  uint *local_sum = malloc(num_threads*sizeof(uint));
  cilk_for(uint i = 0; i < num_threads; i++) {
    uint  ll = i*chk, ul = ll + chk;
    if(i == num_threads-1)
      ul = size;

    uint acc = 0, tmp=0;
    for(uint j = ll; j < ul; j++) {
      tmp = A[j];
      A[j] = acc;
      acc += tmp;
    }
    local_sum[i] = acc;
  }
  
  for(uint i = 1; i < num_threads; i++)
    local_sum[i] += local_sum[i-1];
   
  cilk_for(uint i = 1; i < num_threads; i++) {
    uint ll = i*chk, ul = ll + chk;
    if(i == num_threads-1)
      ul = size;

    uint acc = local_sum[i-1];
    for(uint j = ll; j < ul; j++) {
      A[j] += acc;
    }
  }
  
  return local_sum[num_threads-1];
}

// Add c to each element of A
void parallel_add(uint* A, uint size, uint c) {
  cilk_for(uint i = 0; i < size; i++) {
      A[i] += c;
  }
}

// Fill the table that counts the number of symbols per chunk
uint **fill_table(uint *A, int n, int alphabet, int n_chunks) {
  uint ** count_tbl = (uint **)malloc(alphabet*sizeof(uint *));
  for(int i=0; i<alphabet; i++)
    count_tbl[i] = (uint *)calloc(n_chunks,sizeof(uint));

  uint chk = n/n_chunks;
  cilk_for(uint i = 0; i < n_chunks; i++) {
    uint  ll = i*chk, ul = ll + chk;
    if(i == n_chunks-1)
      ul = n;
    
    for(uint j = ll; j < ul; j++)
      count_tbl[A[j]][i]++;
  }
  return count_tbl;
}

// Write each element of A in the corresponding position in B
uint *write_sorted_array(uint *A, int n, int alphabet, int n_chunks, uint
			  **count_tbl) {
  uint *B = (uint *)malloc(n*sizeof(uint));

  uint chk = n/n_chunks;
  cilk_for(uint i = 0; i < n_chunks; i++) {
    uint  ll = i*chk, ul = ll + chk;
    if(i == n_chunks-1)
      ul = n;

    uint pos = 0;
    for(uint j = ll; j < ul; j++) {
      pos = count_tbl[A[j]][i];
      B[pos] = A[j];
      count_tbl[A[j]][i]++;
    }
  }
  return B;
}

uint * parallel_counting_sort(uint *A, int n, int alphabet) {
  int n_chunks = num_threads;
  uint **count_tbl;
  uint *total_symbol = malloc(alphabet*sizeof(uint));
 
  count_tbl = fill_table(A, n, alphabet, n_chunks);

  for(int i=0; i<alphabet; i++)
    total_symbol[i] = parallel_prefix_sum(count_tbl[i], n_chunks);

  parallel_prefix_sum(total_symbol, alphabet);
  
  for(int i = 1; i < alphabet; i++)
    parallel_add(count_tbl[i], n_chunks, total_symbol[i]);

  uint *B = write_sorted_array(A, n, alphabet, n_chunks, count_tbl);

  return B;
}

uint *generate_array(int n, int alphabet) {
  srand(256);
  uint *x = (uint *)malloc(n*sizeof(uint));
  
  for (int i = 0; i < n; i++)
    x[i] = rand() % alphabet;

  return x;
}

void print_array(int *x, int n) {
  for (int i = 0; i < n; i++)
    fprintf(stderr, "%u ", x[i]);
  fprintf(stderr, "\n");
}

int main(int argc, char* argv[]) {
  
  if(argc != 3) {
    fprintf(stderr, "Usage: %s <number of elements> <alphabet size>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int n = atoi(argv[1]);
  int alphabet = atoi(argv[2]);
  uint *A = generate_array(n, alphabet);
  
  struct timespec stime, etime;
  double t;
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  // Not inplace
  uint *B = parallel_counting_sort(A, n, alphabet);

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);
  
  return EXIT_SUCCESS;
}
