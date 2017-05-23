#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <stdint.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <omp.h>

#include "dictionary.h"
#include "rlz_tools.hpp"
#include "../../LZscan/algorithm/lzscan.h"

int main(int argc, char **argv) { 
  if (argc != 6) {
    fprintf(stderr, "usage: %s [DICTIONARY] [INPUT] [OUTPUT] [N_CHUNKS] \
  [MAX_MEM_MB]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char * reference_filename = argv[1];
  char * input_filename = argv[2];
  char * output_filename = argv[3];
  size_t n_partitions = (size_t)atoi(argv[4]);
  size_t max_memory_MB = atoi(argv[5]);
    
  FILE * file_out = fopen(output_filename,"w");
  Writer w(file_out);
  
  size_t reference_len = file_size(reference_filename);
  size_t n_factors;

  if (reference_len >= INT32_MAX) {
    std::cerr << "length of the reference must be <: " << INT32_MAX <<
      std::endl;
    return EXIT_FAILURE;
  }
  
  struct timespec stime, etime;
  double t;

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  n_factors = parse_in_external_memory<uint32_t>(input_filename, reference_filename, n_partitions, max_memory_MB, w); 

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

  printf(" Done.\n");
  printf("elapsed time: %lf, number of factors: %ld\n", t, n_factors);

  fflush(stdout);
  fflush(stderr);
  return EXIT_SUCCESS;
}
