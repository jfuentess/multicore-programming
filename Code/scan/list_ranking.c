/*
 * This implementation is based on the algorithm described in
 * https://pdfs.semanticscholar.org/54fb/843f085c978e45868bfabbd819096a060334.pdf 
 *
 * David R. Helman and Joseph JáJá. Prefix computations of symmetric
 * multiprocessors, in Journal of parallel and distributed computing. 2001
 *
 * Compile: gcc -std=gnu99 -o lr list_ranking.c -fcilkplus -lcilkrts -lm -lrt
 * Notes:
 *     - Use -DSEQ to compile the sequential algorithm
 *     - Use -DPAR to compile the parallel algorithm
 *     - Use -DDEBUG to compile a debugging version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct _node_t Node;

struct _node_t {
  uint value;
  int next;
};

Node* read_list(const char* fn, uint* n) {
  FILE *fp = fopen(fn, "r");
  char line_buffer[BUFSIZ]; /* BUFSIZ is defined if you include stdio.h */

  if (!fp) {
    fprintf(stderr, "Error opening file \"%s\".\n", fn);
    exit(EXIT_FAILURE);
  }

  *n = atoi(fgets(line_buffer, sizeof(line_buffer), fp));

  Node* list = (Node*)malloc(*n*sizeof(Node));

  for(uint i=0; i<*n; i++) {
    fgets(line_buffer, sizeof(line_buffer), fp);
    list[i].value = atoi(strtok(line_buffer, " "));
    list[i].next = atoi(strtok(NULL, " "));
  }

  return list;
}

/* Assuming that the node in position 0 is the head of the list */
void sequential_list_ranking(Node* L, uint n) {
  uint cnt = 0;
  uint curr = 0;
  uint tmp = 0, accum = 0;
  while(cnt < n) {
    tmp = L[curr].value;
    L[curr].value = accum;
    accum += tmp;
    curr = L[curr].next;
    cnt++;
  }
}

/* Assuming that the node in position 0 is the head of the list. We also assume
   that the 'next' field will not be longer used again

   Observation: This implementation has a bad behavior for the a list of
   20.000.000 of elements and a jump of 8. It generates a bad case: One thread
   do the 87% of the work. To reproduce this case, generate a linked list of 20
   millions of elements, with a jump of 8, using the algorithm implemented in
   gennerate_list.c
*/
void parallel_list_ranking(Node* L, uint n) {

  struct sublist_node {
    int head;
    int next;
    int scratch;
    int value;
  };

  uint s = ceil(log2(n)*num_threads);
  uint chk = n/s;
  if(s > n) {
    s = n;
    chk = 1;
  }

  struct sublist_node* sublist = malloc(s*sizeof(struct sublist_node));

  // Compute the splitters
  cilk_for(uint i = 0; i < s; i++) {
    uint x = i*chk;
    sublist[i].head = x;
    sublist[i].value = L[x].value;
    sublist[i].next = -1;
    sublist[i].scratch = L[x].next;
    L[x].next = -(i)-1;
  }

#if defined(DEBUG)
  uint *counters = calloc(s, sizeof(uint));
#endif

  cilk_for(uint i = 0; i < s; i++) {
    int curr = sublist[i].scratch;
    uint accum = 0, tmp = 0;

    while(curr > 0) {
      tmp = L[curr].value;
      L[curr].value = accum;
      accum += tmp;

      int aux = L[curr].next;
      L[curr].next = -(i)-1;
      curr = aux;

#if defined(DEBUG)
      counters[i]++;
#endif
    }
    sublist[i].next = -(curr)-1;

    // Special case
    if(curr != 0) {
      sublist[-(curr)-1].value = accum;
    }
  }

#if defined(DEBUG)
  uint min_c = 0;
  uint max_c = 0;
  for(int i=0; i<s; i++) {
    if(i==0) {
      min_c = counters[i];
      max_c = counters[i];
    }
    if(counters[i] > max_c)
      max_c = counters[i];
    else if(counters[i] < max_c)
      min_c = counters[i];
  }
  printf("s: %u\n", s);
  printf("Min: %d\n", min_c);
  printf("Max: %d\n", max_c);
#endif

  int curr = 0;
  int accum = 0, tmp = 0;

  while(1) {
    tmp = sublist[curr].value;
    sublist[curr].value += accum;
    accum += tmp;

    curr = sublist[curr].next;
    if(curr < 0)
      break;
  }

  cilk_for(uint i = 0; i < s; i++) {
    uint  ll = i*chk, ul = ll + chk;
    if(i == s-1)
      ul = n;
    if(i == 0)
      ll++;

    for(uint j = ll; j < ul; j++) {
       int idx = -(L[j].next)-1;
       L[j].value += sublist[idx].value;

    }
  }

  L[0].value=0;
  free(sublist);
}

int main(int argc, char* argv[]) {

  if(argc < 2) {
    fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  struct timespec stime, etime;
  double t;

  uint n=0;
  Node *list = read_list(argv[1], &n);
  Node *list2 = read_list(argv[1], &n);


  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

#if defined(SEQ)
  sequential_list_ranking(list, n);
#elif defined(PAR)
  parallel_list_ranking(list2, n);
#endif

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  printf("%d,%s,%lf,%u,%s\n", __cilkrts_get_nworkers(), argv[0], t, n, argv[1]);

  return EXIT_SUCCESS;
}
