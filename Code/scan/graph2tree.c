// Compile: gcc -std=gnu99 -o g2t graph2tree.c defs.c util.c -fcilkplus -lcilkrts

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

#include "util.h"

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

void tree_traversal(Graph *g, uint init, uint *marked_edges) {
  char *visited = (char *)calloc(g->n,sizeof(char));
  
  Stack *s = stack_create(g->n);
  visited[init] = 1;
  stack_push(s, init);
  
  while(!stack_empty(s)) {
    uint curr = (uint)stack_pop(s);
    
    for(uint i = g->V[curr].first; i <= g->V[curr].last; i++) {
      uint tgt = g->E[i].tgt;
      
      if(!visited[tgt]) { // Not visited
	visited[tgt] = 1;
	stack_push(s, tgt);
	marked_edges[i] = 1;
	marked_edges[g->E[i].cmp] = 1;
      }
    }
  }
  
  free(visited);
  stack_free(s);
}
      
int main(int argc, char* argv[]) {

  if(argc < 2) {
    fprintf(stderr, "Usage: %s <input graph>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  struct timespec stime, etime;
  double t;
  
  Graph *g = read_graph_from_file(argv[1]);
  uint *marked_edges = calloc(2*g->m, sizeof(uint));
  tree_traversal(g, 0, marked_edges);

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  /* YOUR PARALLEL CODE */

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);
  
  return EXIT_SUCCESS;
}
