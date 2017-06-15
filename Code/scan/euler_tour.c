// Compile: gcc -std=gnu99 -o et euler_tour.c defs.c util.c -fcilkplus -lcilkrts -lrt -lm

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

#include "util.h"
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
#endif

#define num_threads __cilkrts_get_nworkers()

void parallel_list_ranking(ENode* L, uint n) {

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
    }
    sublist[i].next = -(curr)-1;

    // Special case
    if(curr != 0) {
      sublist[-(curr)-1].value = accum;
    }
  }

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

uint* euler_tour(Tree *t, uint init) {
  uint num_edges = 2*(t->n-1);
  ENode* ET = malloc(num_edges*sizeof(ENode)); // Euler Tour represented by a linked list
  uint* output = malloc(num_edges*sizeof(uint)); // Euler Tour represented by an array

  uint chk = num_edges/num_threads;

  cilk_for(uint h = 0; h < num_threads; h++) {
    uint ll = h*chk;
    uint ul = ll+chk;
    if(h == num_threads-1)
      ul = num_edges;

    for(uint i = ll; i < ul; i++) {
      Edge e = t->E[i];
      Node n_src = t->N[e.src];
      Node n_tgt = t->N[e.tgt];

      ET[i].value = 1;

      /* Forward edges. In the root, all the edges are forward edges */
      /* e.src == init means the root n_src.first != i means a forward edge */
      if((e.src == init) || n_src.first != i) {
	
	/* Leaf */
  	if(n_tgt.first == n_tgt.last)
  	  ET[i].next = e.cmp;
  	/* Connect to the first child of the tgt node */
  	else
  	  ET[i].next = n_tgt.first+1;
      }
      /* Backward edges. The root has not backward edges */
      else {
  	/* Especial case for the last child of the root */
  	if(e.tgt == init && n_tgt.last == e.cmp)
  	  ET[i].next = 0;
  	else {
  	  uint last = n_tgt.last;
	  
  	  /* Last child of n_tgt */
  	  if(e.cmp == last)
  	    ET[i].next = n_tgt.first;
  	  /* Intermediate child */
  	  else
  	    ET[i].next = e.cmp + 1;
  	}
      }
    }
  }

  parallel_list_ranking(ET, 2*(t->n-1));

  cilk_for(uint h = 0; h < num_threads; h++) {
    uint ll = h*chk;
    uint ul = ll+chk;
    if(h == num_threads-1)
      ul = num_edges;
    
    for(uint i = ll; i < ul; i++)
      output[ET[i].value] = t->E[i].src;
  }

  return output;
}


int main(int argc, char* argv[]) {

  if(argc < 2) {
    fprintf(stderr, "Usage: %s <input graph>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  struct timespec stime, etime;
  double t;
  
  Tree *tree = read_tree_from_file(argv[1]);

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  uint *ET = euler_tour(tree, 0);
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("threads: %d, elapsed time: %lf, tree: %s\n", __cilkrts_get_nworkers(), t, argv[1]);
  
  return EXIT_SUCCESS;
}
