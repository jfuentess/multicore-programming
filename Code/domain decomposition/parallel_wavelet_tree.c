#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
#include <cilk/reducer_opadd.h>
#endif

#define num_threads __cilkrts_get_nworkers()

#include "basic_wt.h"
#include "util.h"

#define FACTOR 20 // bitmap usage

unsigned long*** counters;
unsigned long*** lo;
unsigned long*** lo2;

void prefix_sum_body(unsigned int idx) {
    unsigned int nnum = 1 << idx;
    unsigned long* z = (unsigned long*)malloc((num_threads-1)*sizeof(unsigned long));
    unsigned int k = 0;

    cilk_for(k = 0; k < num_threads; k++) {
      unsigned long llimit = k*nnum, ulimit = llimit + nnum;
      unsigned long thread, node, aux = 0;
      unsigned long l = 0;
      for(l = llimit; l < ulimit; l++) {
	thread = l%num_threads;	
	node = l/num_threads;
	counters[thread][idx][node] = aux;
    	aux += lo[thread][idx][node];	
      }      
    }

    unsigned long aux = 0;
    for(k = 0; k < num_threads-1; k++) {
      unsigned long pos = (k+1)*nnum-1;
      aux += lo[pos%num_threads][idx][pos/num_threads];
      z[k] = aux;
    }

    for(k = 1; k < num_threads; k++) {
      unsigned long pos = (k+1)*nnum-1;
      counters[pos%num_threads][idx][pos/num_threads] += counters[(pos-nnum)%num_threads][idx][(pos-nnum)/num_threads];
    }

    cilk_for(k = 1; k < num_threads; k++) {
      unsigned long llimit = k*nnum;
      unsigned long ulimit = llimit + nnum;
      unsigned long thread;
      unsigned long node;
      unsigned long aux = 0;
      unsigned long t = (llimit-1)%num_threads;
      unsigned long nd = (llimit-1)/num_threads;
      unsigned long l = 0;
      for(l = llimit; l < ulimit-1; l++){
    	thread = l%num_threads;
    	node = l/num_threads;
	counters[thread][idx][node] += counters[t][idx][nd]+z[k-1];
      }
    }

    cilk_for(k = 1; k < num_threads; k++) {
      unsigned long pos = (k+1)*nnum-1;
      counters[pos%num_threads][idx][pos/num_threads] += z[k-1];
    }
}


void prefix_sum(unsigned int alphabet) {
  unsigned int levels = log2(alphabet);
  unsigned int i = 0;

  for(i = 0; i < levels; i++) {
    cilk_spawn prefix_sum_body(i);
  }
  cilk_sync;

  return;
}

/*
void prefix_sum(unsigned int alphabet){
  unsigned int levels = log2(alphabet);
  unsigned int i = 0;
  cilk_for(i = 0; i < levels; i++) {
    
    unsigned int nnum = 1 << i;
    unsigned long* z = (unsigned long*)malloc((num_threads-1)*sizeof(unsigned long));
    unsigned int k = 0;
    // +++
    for(k = 0; k < num_threads; k++) {
      unsigned long llimit = k*nnum, ulimit = llimit + nnum;
      unsigned long l = 0;
      unsigned long thread, node, aux = 0;

      //      printf("llimit: %lu, ulimit: %lu (%d)\n", llimit, ulimit, __cilkrts_get_worker_number());
      for(l = llimit; l < ulimit; l++) {
	thread = l%num_threads;	
	node = l/num_threads;
	counters[thread][i][node] = aux;
    	aux += lo[thread][i][node];	
      }      
    }
    
    unsigned long aux = 0;
    for(k = 0; k < num_threads-1; k++) {
      unsigned long pos = (k+1)*nnum-1;
      aux += lo[pos%num_threads][i][pos/num_threads];
      z[k] = aux;
    }

    for(k = 1; k < num_threads; k++) {
      unsigned long pos = (k+1)*nnum-1;
      counters[pos%num_threads][i][pos/num_threads] += counters[(pos-nnum)%num_threads][i][(pos-nnum)/num_threads];
    }

    // +++ 
    for(k = 1; k < num_threads; k++) {
      unsigned long llimit = k*nnum;
      unsigned long ulimit = llimit + nnum;
      unsigned long thread;
      unsigned long node;
      unsigned long aux = 0;
      unsigned long t = (llimit-1)%num_threads;
      unsigned long nd = (llimit-1)/num_threads;

      unsigned long l = 0;
      for(l = llimit; l < ulimit-1; l++){
    	thread = l%num_threads;
    	node = l/num_threads;
	counters[thread][i][node] += counters[t][i][nd]+z[k-1];
      }
    }

    // +++
    for(k = 1; k < num_threads; k++) {
      unsigned long pos = (k+1)*nnum-1;
      counters[pos%num_threads][i][pos/num_threads] += z[k-1];
    }
    
  }
  return;
}
    */
// Merge the partial wavelet trees into one wavelet tree
#ifdef NO_RANK_SELECT
BIT_ARRAY** _merge_wt(BIT_ARRAY*** partial_wtree, unsigned long n, unsigned int alphabet) {
#else
bitRankW32Int** _merge_wt(BIT_ARRAY*** partial_wtree, unsigned long n, unsigned int alphabet) {
#endif

  unsigned int i = 0;
  unsigned int levels = log2(alphabet);
  // Final wavelet tree
#ifdef NO_RANK_SELECT
  BIT_ARRAY** wtree = (BIT_ARRAY**)malloc(levels * sizeof(BIT_ARRAY*));
#else
  bitRankW32Int** wtree = (bitRankW32Int**)malloc(levels * sizeof(bitRankW32Int*));
#endif

#ifdef NO_RANK_SELECT
  for(i = 0; i < levels; i++)
    wtree[i] = bit_array_create(n);
#else
  BIT_ARRAY** bitarray = (BIT_ARRAY**)malloc(levels*sizeof(BIT_ARRAY*));
  for(i = 0; i < levels; i++)
    bitarray[i] = bit_array_create(n);
#endif


  cilk_for(i = 0; i < levels; i++) {  
    unsigned int nnum = 1 << i;
    unsigned int k = 0;
    
    cilk_for(k = 0; k < num_threads; k++) {
      unsigned long llimit = k*nnum,  ulimit = llimit + nnum;
      unsigned long thread, node;
      unsigned long l = 0;

      for(l = llimit; l < ulimit; l++) {

	thread = l%num_threads;	
	node = l/num_threads;
	//	counters[thread][i][node];

#ifdef NO_RANK_SELECT
	if(node == nnum-1) {
	  parallel_bit_array_concat_from_to(wtree[i], partial_wtree[thread][i], counters[thread][i][node], lo2[thread][i][node], partial_wtree[thread][i]->num_of_bits-lo2[thread][i][node]);
	}
	else {
	  parallel_bit_array_concat_from_to(wtree[i], partial_wtree[thread][i], counters[thread][i][node], lo2[thread][i][node], lo2[thread][i][node+1]-lo2[thread][i][node]);
	}
#else
	if(node == nnum-1) {
	  parallel_bit_array_concat_from_to(bitarray[i], partial_wtree[thread][i], counters[thread][i][node], lo2[thread][i][node], partial_wtree[thread][i]->num_of_bits-lo2[thread][i][node]);
	  
	}
	else {
	  parallel_bit_array_concat_from_to(bitarray[i], partial_wtree[thread][i], counters[thread][i][node], lo2[thread][i][node], lo2[thread][i][node+1]-lo2[thread][i][node]);
	}
#endif
      }
    }
  }

#ifndef NO_RANK_SELECT
  for(i = 0; i < levels; i++) {
#ifdef ARCH64
      fprintf(stderr, "createBitRankW32Int() needs to be adapted to work on 64-bits mode");
      exit(1);
#endif
      // Create the rank/select structures
      wtree[i] = createBitRankW32Int(bitarray[i]->words, bitarray[i]->num_of_bits, 1, FACTOR);
  }
#endif


  return wtree;
}

// Create the wavelet tree of a subsequence
BIT_ARRAY** _wt_create(symbol* T, unsigned long n, unsigned int alphabet, int tid) {
  unsigned int levels = log2(alphabet);

  BIT_ARRAY** wtree = (BIT_ARRAY**)malloc(sizeof(BIT_ARRAY*) * levels);

  unsigned int nnum = 0, i = 0;
  for (i = 0; i < levels; i++) {
    // Number of nodes in the i-th level
    nnum = 1 << i;

    // Allocating memory to save the number of symbols on each node of the
    // partial wavelet tree
    counters[tid][i] = (unsigned long*)calloc(nnum, sizeof(unsigned long));
    lo[tid][i] = (unsigned long*)calloc(nnum, sizeof(unsigned long));
    lo2[tid][i] = (unsigned long*)calloc(nnum, sizeof(unsigned long));
    wtree[i] = bit_array_create(n);

    symbol k;
    unsigned int schunk = 0;
    unsigned int desp1 = levels - i, desp2 = desp1 - 1;
    unsigned long j = 0;
    // Counting the number of symbols on each node
    for (j = 0; j < n; j++) {
      k = T[j];
      schunk = (unsigned int)k >> desp1;
      counters[tid][i][schunk]++;
    }

    // Offsets
    unsigned int cnt = 0, aux = 0, z = 0;
    for(z = 0; z < nnum; z++) {
      aux = counters[tid][i][z];
      lo[tid][i][z] = counters[tid][i][z];
      lo2[tid][i][z] = cnt;
      counters[tid][i][z] = cnt;
      cnt += aux;
    }
    
    // Filling the bit arrays
    for (j = 0; j < n; j++) {
      k = T[j];
      schunk = (unsigned int)k >> desp1;
      k &= 1 << desp2;

      if (k)
	bit_array_set_bit(wtree[i], counters[tid][i][schunk]);

      counters[tid][i][schunk]++;
    }
  }

  return wtree;
}

// Create the wavelet tree of the input sequence T, over the alphabet 'alphabet'
#ifdef NO_RANK_SELECT
BIT_ARRAY** wt_create(symbol* T, unsigned long n, unsigned int alphabet) {
#else
bitRankW32Int** wt_create(symbol* T, unsigned long n, unsigned int alphabet) {
#endif

  // Number of symbols that each thread will process
  unsigned long segment_size = n / num_threads;

  unsigned int levels = log2(alphabet);
  // Number of bits in each node of all partial wavelet trees
  counters = (unsigned long***)malloc(sizeof(unsigned long**) * num_threads);
  lo = (unsigned long***)malloc(sizeof(unsigned long**) * num_threads);
  lo2 = (unsigned long***)malloc(sizeof(unsigned long**) * num_threads);

  // DOMAIN DECOMPOSITION SECTION
  BIT_ARRAY** partial_wtree[num_threads];
  unsigned int i = 0;
  cilk_for(i=0; i < num_threads; i++) {
    counters[i] = (unsigned long**)malloc(sizeof(unsigned long*) * levels);
    lo[i] = (unsigned long**)malloc(sizeof(unsigned long*) * levels);
    lo2[i] = (unsigned long**)malloc(sizeof(unsigned long*) * levels);

    if(i == (num_threads - 1) && n % (num_threads * segment_size) != 0)
      partial_wtree[i] = _wt_create(&T[i * segment_size], (n - i * segment_size), alphabet, i);
    else
      partial_wtree[i] = _wt_create(&T[i * segment_size], segment_size, alphabet, i);
  }

/*  struct timespec stime, etime;
  double t;
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
*/
  prefix_sum(alphabet);

  // MERGE SECTION
#ifdef NO_RANK_SELECT
  //BIT_ARRAY** wtree = NULL;
  BIT_ARRAY** wtree = _merge_wt(partial_wtree, n, alphabet);
#else
  //bitRankW32Int** wtree = NULL;
  bitRankW32Int** wtree = _merge_wt(partial_wtree, n, alphabet);
#endif
/*
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  printf("%lf", t);
*/
  return wtree;
}


unsigned int _wt_rank_0(bitRankW32Int* level, unsigned int idx) {
  return ((1 + idx) - rank(level, idx));
}

unsigned int _wt_rank_1(bitRankW32Int* level, unsigned int idx) {
  return rank(level, idx);
}

unsigned int wt_access(bitRankW32Int** wtree, unsigned int idx, unsigned int
alphabet) {
  unsigned int lls = 0; // Lower limit of the sequence
  unsigned int uls = lenght_in_bits(wtree[0]) - 1; ; // Upper limit of the sequence

  int levels = log2(alphabet);
  unsigned int lla = 0; // Lower limit of the alphabet
  unsigned int ula = alphabet - 1; // Upper limit of the alphabet

  unsigned int crank = 0;
  unsigned int partial_crank = 0;

  unsigned int i = 0;
  for (i = 0; i < levels; i++) {
    if(isBitSet(wtree[i], idx + lls) != 1) {
      crank = _wt_rank_0(wtree[i], lls + idx) - _wt_rank_0(wtree[i], lls-1);
      idx = crank - 1;
      ula = (lla + ula) / 2;
      uls -= _wt_rank_1(wtree[i], uls) - _wt_rank_1(wtree[i], lls-1);

    } else {
      crank = _wt_rank_1(wtree[i], lls + idx) - _wt_rank_1(wtree[i], lls - 1);
      idx = crank - 1;
      lla = ((lla + ula) / 2) + 1;
      lls += _wt_rank_0(wtree[i], uls) - _wt_rank_0(wtree[i], lls - 1);
    }
  }
  return lla;
}

// Print the wavelet tree
void wtree_to_string(bitRankW32Int** wtree, unsigned int alphabet) {
  // Number of levels of the wavelet tree
  unsigned int levels = log2(alphabet);

  unsigned int i = 0;
  for (i = 0; i < levels; i++) {
    BIT_ARRAY p;
    p.words = wtree[i]->data;
    p.num_of_bits = wtree[i]->n;

    char* st = bit_array_to_string(&p);
    printf("%s\n", st);
    free(st);
  }
}

int main(int argc, char* argv[]) {

  if(argc != 3 && argc != 4){
    fprintf(stderr, "Execute: %s <input file> <alphabet size> [<validation_file>]\n", argv[0]);
    exit(-1);
  }

  unsigned long n; // Size of the input sequence
  symbol* text = read_text_from_file(argv[1], &n); // Input sequence
  unsigned int alphabet = (unsigned int)atoi(argv[2]); // Size of the alphabet

  //printf("n: %lu, alphabet: %u, threads: %d\n", n, alphabet, num_threads);

//  printf("%s,%lu,%u,", argv[1], n, alphabet);

  // Memory usage
#ifdef MALLOC_COUNT
  size_t s_total_memory = malloc_count_total();
  size_t s_current_memory = malloc_count_current();
  malloc_reset_peak();

  // Running time. CLOCK_THREAD_CPUTIME_ID: Running time of the thread that call it (main thread in this case)
#else
  struct timespec stime, etime;
  double t;
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
#endif

  // Wavelet tree construction
#ifdef NO_RANK_SELECT
  BIT_ARRAY** wtree = wt_create(text, n, alphabet);
#else
  bitRankW32Int** wtree = wt_create(text, n, alphabet);
#endif

#ifdef MALLOC_COUNT
  size_t e_total_memory = malloc_count_total();
  size_t e_current_memory = malloc_count_current();
  printf("%s, %u, %zu, %zu, %zu, %zu, %zu\n", argv[1], alphabet, s_total_memory, e_total_memory, malloc_count_peak(), s_current_memory, e_current_memory);

#else
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  printf("%d,%s,%lu,%lf\n", __cilkrts_get_nworkers(), argv[1], n, t);
//  printf("%d,%s,%lu\n", __cilkrts_get_nworkers(), argv[1], n); // Merge
#endif

  free(text);

  // Validation mode
  // Generate the original text using the wavelet tree
  if(argc > 3) {
#ifdef NO_RANK_SELECT
    printf("Compile without -DNO_RANK_SELECT to use the validation mode\n");
#else
    FILE *test_file;
    test_file = fopen(argv[3],"wb");

    if (!test_file) {
      printf("Unable to open file!");
      return EXIT_FAILURE;
    }

    symbol access_return = 0;
    unsigned long i = 0;

    if(n*sizeof(symbol) > 10485760)
      printf("Please, use a file smaller than 10MB to validate the algorithm. Otherwise, the validation could take a while.\n");
    else {
      for(i = 0; i < n; i++) {
	access_return =  wt_access(wtree, i, alphabet);
	fwrite(&access_return, sizeof(symbol), 1, test_file);
      }
    }
    fclose(test_file);
#endif
  }

  return EXIT_SUCCESS;
}
