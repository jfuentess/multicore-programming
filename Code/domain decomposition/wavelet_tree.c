#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

#include "bitrank/bitrankw32int.h"
#include "bitrank/bit_array.h"

#define num_threads __cilkrts_get_nworkers()

#define FACTOR 20 // bitmap usage

typedef unsigned int symbol;

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

BIT_ARRAY** compute_bitarrays(symbol* T, unsigned long n, unsigned int alphabet) {

  unsigned int levels = log2(alphabet);

  BIT_ARRAY** bit_arrays = (BIT_ARRAY**)malloc(levels * sizeof(BIT_ARRAY*));

  for (uint i = 0; i < levels; i++) {
    // Number of nodes in the i-th level
    unsigned int nnum = 1 << i;

    // Allocating memory to save the number of symbols on each node of the wavelet tree
    unsigned int* counters = (unsigned int*)calloc(nnum, sizeof(unsigned int));
    bit_arrays[i] = bit_array_create(n);

    symbol k;
    unsigned int schunk = 0;
    unsigned int desp1 = levels - i, desp2 = desp1 - 1;

    // Counting the number of symbols on each node
    for (uint j = 0; j < n; j++) {
      k = T[j];
      schunk = (unsigned int)k >> desp1;
      counters[schunk]++;
    }

    // Offsets
    unsigned int cnt = 0, aux = 0;
    for(uint z = 0; z < nnum; z++) {
      aux = counters[z];
      counters[z] = cnt;
      cnt += aux;
    }

    // Filling the bit arrays
    for (uint j = 0; j < n; j++) {
      k = T[j];
      schunk = (unsigned int)k >> desp1;
      k &= 1 << desp2;
      
      if (k)
        bit_array_set_bit(bit_arrays[i], counters[schunk]);
      counters[schunk]++;
    }

    free(counters);
  }

  return bit_arrays;
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
  BIT_ARRAY** bit_arrays = compute_bitarrays(text, n, alphabet);
  bitRankW32Int** wtree = rank_select_wt(bit_arrays, alphabet);

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  printf("%d,%s,%lu,%lf\n", 1, argv[1], n, t);

  uint ii = 9, sym = 156;
  printf("rank(%u): %u\n", ii, wt_rank(wtree, alphabet, ii, sym));

  free(text);

  return EXIT_SUCCESS;
}
