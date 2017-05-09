#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

#include "bitrank/bitrankw32int.h"
#include "bitrank/bit_array.h"

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

#define FACTOR 20 // bitmap usage

typedef unsigned int symbol;

unsigned long*** counters;
unsigned long*** lo; // local offsets
unsigned long*** go; // global offsets

symbol* read_text_from_file(const char* fn, unsigned long* n) {

  FILE* fp = fopen(fn, "r");
  size_t read;

  if (!fp) {
    fprintf(stderr, "Error opening file \"%s\".\n", fn);
    exit(-1);
  }

  fseek(fp, 0L, SEEK_END);
  *n = (unsigned long)ftell(fp);

  symbol* t;
  t = malloc(*n);

  *n = *n / sizeof(symbol); // Number of symbols

  fseek(fp, 0L, SEEK_SET);

  read = fread(t, sizeof(symbol), *n, fp);
  if(read != *n){
    fprintf(stderr, "Error reading file \"%s\".\n", fn);
    exit(-1);
  }

  fclose(fp);

  return t;

}

bitRankW32Int** rank_select_wt(BIT_ARRAY** bit_arrays, unsigned int alphabet) {
  unsigned int levels = log2(alphabet);
  bitRankW32Int** wtree = (bitRankW32Int**)malloc(levels * sizeof(bitRankW32Int*));

  for (unsigned int i = 0; i < levels; i++) {
    wtree[i] = createBitRankW32Int(bit_arrays[i]->words,
				   bit_arrays[i]->num_of_bits, 1, FACTOR);
  }

  return wtree;
}

unsigned int _wt_rank_0(bitRankW32Int* level, unsigned int idx) {
  return ((1 + idx) - rank(level, idx));
}

unsigned int _wt_rank_1(bitRankW32Int* level, unsigned int idx) {
  return rank(level, idx);
}

uint wt_access(bitRankW32Int** wtree, uint idx, uint alphabet) {
  uint lls = 0; // Lower limit of the sequence
  uint uls = lenght_in_bits(wtree[0]) - 1; ; // Upper limit of the sequence

  int levels = log2(alphabet);
  uint lla = 0; // Lower limit of the alphabet
  uint ula = alphabet - 1; // Upper limit of the alphabet

  uint crank = 0;

  for (uint i = 0; i < levels; i++) {
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

uint wt_rank(bitRankW32Int** wtree, uint alphabet, uint idx, symbol sym) {
  uint lls = 0; // Lower limit of the sequence
  uint uls = lenght_in_bits(wtree[0]) - 1; ; // Upper limit of the sequence

  int levels = log2(alphabet);
  uint crank = 0;

  symbol k;
  for (uint i = 0; i < levels; i++) {
    uint desp = levels - i - 1;
    k = sym & (1 << desp);

    if(k) {
      crank = _wt_rank_1(wtree[i], lls + idx) - _wt_rank_1(wtree[i], lls - 1);
      idx = crank - 1;
      lls += _wt_rank_0(wtree[i], uls) - _wt_rank_0(wtree[i], lls - 1);
    } else {
      crank = _wt_rank_0(wtree[i], lls + idx) - _wt_rank_0(wtree[i], lls-1);
      idx = crank - 1;
      uls -= _wt_rank_1(wtree[i], uls) - _wt_rank_1(wtree[i], lls-1);
    }    
  }
  
  k = sym & 1;
  if(k)
    crank = _wt_rank_1(wtree[levels-1], lls + idx) - _wt_rank_1(wtree[levels-1],
								lls - 1);
  else
    crank = _wt_rank_0(wtree[levels-1], lls + idx) -
      _wt_rank_0(wtree[levels-1], lls-1);
  
  return crank;
}

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

BIT_ARRAY** compute_bitarrays(symbol* T, ulong n, uint alphabet, int tid) {

  unsigned int levels = log2(alphabet);

  BIT_ARRAY** bit_arrays = (BIT_ARRAY**)malloc(levels * sizeof(BIT_ARRAY*));

  for (uint i = 0; i < levels; i++) {
    // Number of nodes in the i-th level
    unsigned int nnum = 1 << i;

    // Allocating memory to save the number of symbols on each node of the
    // partial wavelet tree
    counters[tid][i] = (unsigned long*)calloc(nnum, sizeof(unsigned long));
    lo[tid][i] = (unsigned long*)calloc(nnum, sizeof(unsigned long));
    go[tid][i] = (unsigned long*)calloc(nnum, sizeof(unsigned long));
    bit_arrays[i] = bit_array_create(n);

    symbol k;
    unsigned int schunk = 0;
    unsigned int desp1 = levels - i, desp2 = desp1 - 1;

    // Counting the number of symbols on each node
    for (uint j = 0; j < n; j++) {
      k = T[j];
      schunk = (unsigned int)k >> desp1;
      counters[tid][i][schunk]++;
    }

    // Offsets
    unsigned int cnt = 0, aux = 0;
    for(uint z = 0; z < nnum; z++) {
      aux = counters[tid][i][z];
      lo[tid][i][z] = counters[tid][i][z];
      go[tid][i][z] = cnt;
      counters[tid][i][z] = cnt;
      cnt += aux;
    }

    // Filling the bit arrays
    for (uint j = 0; j < n; j++) {
      k = T[j];
      schunk = (unsigned int)k >> desp1;
      k &= 1 << desp2;
      
      if (k)
        bit_array_set_bit(bit_arrays[i], counters[tid][i][schunk]);
      counters[tid][i][schunk]++;
    }
  }

  return bit_arrays;
}

// Merge the partial wavelet trees into one wavelet tree
BIT_ARRAY** _merge_wt(BIT_ARRAY*** partial_wtree, unsigned long n, unsigned int alphabet) {

  unsigned int i = 0;
  unsigned int levels = log2(alphabet);
  BIT_ARRAY** wtree = (BIT_ARRAY**)malloc(levels * sizeof(BIT_ARRAY*));

  for(i = 0; i < levels; i++)
    wtree[i] = bit_array_create(n);

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

	if(node == nnum-1) {
	  parallel_bit_array_concat_from_to(wtree[i], partial_wtree[thread][i], counters[thread][i][node], go[thread][i][node], partial_wtree[thread][i]->num_of_bits-go[thread][i][node]);
	}
	else {
	  parallel_bit_array_concat_from_to(wtree[i], partial_wtree[thread][i], counters[thread][i][node], go[thread][i][node], go[thread][i][node+1]-go[thread][i][node]);
	}
      }
    }
  }
  return wtree;
}

// Create the wavelet tree of the input sequence T, over the alphabet 'alphabet'
BIT_ARRAY** par_wt_create(symbol* T, unsigned long n, unsigned int alphabet) {

  // Number of symbols that each thread will process
  unsigned long segment_size = n / num_threads;

  unsigned int levels = log2(alphabet);

  // Number of bits in each node of all partial wavelet trees
  counters = (unsigned long***)malloc(sizeof(unsigned long**) * num_threads);
  lo = (unsigned long***)malloc(sizeof(unsigned long**) * num_threads);
  go = (unsigned long***)malloc(sizeof(unsigned long**) * num_threads);

  // DOMAIN DECOMPOSITION SECTION
  BIT_ARRAY** partial_wtree[num_threads];
  unsigned int i = 0;
  
  cilk_for(i=0; i < num_threads; i++) {
    counters[i] = (unsigned long**)malloc(sizeof(unsigned long*) * levels);
    lo[i] = (unsigned long**)malloc(sizeof(unsigned long*) * levels);
    go[i] = (unsigned long**)malloc(sizeof(unsigned long*) * levels);

    if(i == (num_threads - 1) && n % (num_threads * segment_size) != 0)
      partial_wtree[i] = compute_bitarrays(&T[i * segment_size], (n - i * segment_size), alphabet, i);
    else
      partial_wtree[i] = compute_bitarrays(&T[i * segment_size], segment_size, alphabet, i);
  }

  prefix_sum(alphabet);

  // MERGE SECTION
  BIT_ARRAY** wtree = _merge_wt(partial_wtree, n, alphabet);

  return wtree;
}

int main(int argc, char* argv[]) {

  if(argc != 3){
    fprintf(stderr, "Execute: %s <input file> <alphabet size>\n", argv[0]);
    exit(-1);
  }

  unsigned long n; // Size of the input sequence
  symbol* text = read_text_from_file(argv[1], &n); // Input sequence
  unsigned int alphabet = (unsigned int)atoi(argv[2]); // Size of the alphabet

  struct timespec stime, etime;
  double t;
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  // Wavelet tree construction
  BIT_ARRAY** bit_arrays = par_wt_create(text, n, alphabet);
  bitRankW32Int** wtree = rank_select_wt(bit_arrays, alphabet);

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  printf("%d,%s,%lu,%lf\n", num_threads, argv[1], n, t);

  free(text);

  uint ii = 9, sym = 156;
  printf("rank(%u): %u\n", ii, wt_rank(wtree, alphabet, ii, sym));

  return EXIT_SUCCESS;
}
