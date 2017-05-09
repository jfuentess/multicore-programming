// Compilar: gcc -std=gnu99 -o sm string_matching.c -fcilkplus -lcilkrts
/*
 * Using an Intel Cilk Plus Reducer in C:
 * https://software.intel.com/en-us/articles/using-an-intel-cilk-plus-reducer-in-c 
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <math.h>

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

CILK_C_REDUCER_OPADD(num_occs, int, 0);

char* read_text(const char* fn, uint* n) {

  FILE* fp = fopen(fn, "r");
  size_t read;

  if (!fp) {
    fprintf(stderr, "Error opening file \"%s\".\n", fn);
    exit(-1);
  }

  fseek(fp, 0L, SEEK_END);
  *n = (uint)ftell(fp);

  char* t;
  t = malloc(*n);

  fseek(fp, 0L, SEEK_SET);

  read = fread(t, sizeof(char), *n, fp);
  if(read != *n){
    fprintf(stderr, "Error reading file \"%s\".\n", fn);
    exit(-1);
  }

  fclose(fp);

  return t;
}

int string_matching(char* text, uint n, char* pattern, uint m) {  
  uint limit = n-m;
  cilk_for(uint i = 0; i < limit; i++) {
    for(uint j = 0; j <= m; j++) {
      if(j==m && pattern[j] == '\0') { // Match
	REDUCER_VIEW(num_occs) += 1;
	break;
      }
      if(text[i+j] != pattern[j]) // No match
	break;
    }
  }

  return num_occs.value;
}

int main(int argc, char* argv[]) {
  
  struct timespec stime, etime;
  double t;

  if(argc != 3){
    fprintf(stderr, "Usage: %s <input file> <pattern>\n", argv[0]);
    exit(-1);
  }

  uint n = 0;
  char* text = read_text(argv[1], &n);
  char* pattern = argv[2];
  uint m = strlen(pattern);
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  CILK_C_REGISTER_REDUCER(num_occs);
  int occ = string_matching(text, n, pattern, m);
  CILK_C_UNREGISTER_REDUCER(num_occs);
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("%s,%d,%lf\n", argv[0], __cilkrts_get_nworkers(), t);
  fprintf(stderr, "%d ocurrences\n", occ);
  
  return EXIT_SUCCESS;
}
