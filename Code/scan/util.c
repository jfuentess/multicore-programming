#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

Tree* read_tree_from_file(const char* fn) {
  Tree *t = malloc(sizeof(Tree));

  FILE *fp = fopen(fn, "r");
  char line_buffer[BUFSIZ]; /* BUFSIZ is defined if you include stdio.h */

  if (!fp) {
    fprintf(stderr, "Error opening file \"%s\".\n", fn);
    exit(EXIT_FAILURE);
  }

  t->n = atoi(fgets(line_buffer, sizeof(line_buffer), fp));

  t->N = calloc(t->n,sizeof(Node));
  t->E = calloc(2*(t->n-1),sizeof(Edge));

  unsigned int i = 0;
  for(i = 0; i < 2*(t->n-1); i++)
    t->E[i].cmp = -1;
  
  unsigned int source = 0, target = 0, m = 0;

  while (fgets(line_buffer, sizeof(line_buffer), fp)) {
    source = atoi(strtok(line_buffer, " "));
    target = atoi(strtok(NULL, " "));
    t->E[m].src = source;
    t->E[m].tgt = target;

    if(m==0)
      t->N[source].first = m;
    else if(source != t->E[m-1].src) {
      t->N[t->E[m-1].src].last = m-1;
      t->N[source].first = m;
    }
    m++;
  }
  t->N[t->E[m-1].src].last = m-1;

  fclose(fp);

  for(i = 0; i < 2*(t->n-1); i++) {
    Node target = t->N[t->E[i].tgt];
    int cmp = -1;

    if(t->E[i].cmp != -1)
      continue;
    
    unsigned int j = 0;
    for(j = target.first; j <= target.last; j++) {
      if((t->E[j].cmp == -1) && (t->E[j].tgt == t->E[i].src))
	cmp = j;
    }

    if(cmp != -1) {
      t->E[i].cmp = cmp;
      t->E[cmp].cmp = i;
    }
  }

  return t;
}

void write_tree_to_file(const char *fn, Tree *t) {

  FILE* fp = fopen(fn, "w");

  if (!fp) {
    fprintf(stderr, "Error opening file \"%s\".\n", fn);
    exit(EXIT_FAILURE);
  }

  unsigned int i = 0, j = 0;

  fprintf(fp, "%u\n", t->n);

  for(i = 0; i < t->n; i++)
    for(j = t->N[i].first; j <= t->N[i].last; j++)
      fprintf(fp, "%u %u\n", t->E[j].src, t->E[j].tgt);

  fclose(fp);
}

void free_tree(Tree* t) {
  free(t->N);
  free(t->E);
  free(t);
}

Graph* read_graph_from_file(const char* fn) {
  Graph *g = malloc(sizeof(Graph));

  FILE *fp = fopen(fn, "r");
  char line_buffer[BUFSIZ]; /* BUFSIZ is defined if you include stdio.h */

  if (!fp) {
    fprintf(stderr, "Error opening file \"%s\".\n", fn);
    exit(EXIT_FAILURE);
  }

  g->n = atoi(fgets(line_buffer, sizeof(line_buffer), fp));
  g->m = atoi(fgets(line_buffer, sizeof(line_buffer), fp));

  g->V = calloc(g->n,sizeof(Vertex));
  g->E = calloc(2*(g->m),sizeof(Edge));

  unsigned int i = 0;
  for(i = 0; i < 2*g->m; i++)
    g->E[i].cmp = -1;
  
  unsigned int source = 0, target = 0, m = 0;

  while (fgets(line_buffer, sizeof(line_buffer), fp)) {
    source = atoi(strtok(line_buffer, " "));
    target = atoi(strtok(NULL, " "));
    g->E[m].src = source;
    g->E[m].tgt = target;

    if(m==0)
      g->V[source].first = m;
    else if(source != g->E[m-1].src) {
      g->V[g->E[m-1].src].last = m-1;
      g->V[source].first = m;
    }
    m++;
  }
  g->V[g->E[m-1].src].last = m-1;

  fclose(fp);

  for(i = 0; i < 2*g->m; i++) {
    Vertex target = g->V[g->E[i].tgt];
    int cmp = -1;

    if(g->E[i].cmp != -1)
      continue;
    
    unsigned int j = 0;
    for(j = target.first; j <= target.last; j++) {
      /* To support multiple edges among two vertices, we choose the last */
      /* unvisited (e.cmp=-1) edge, not the first one */
      if((g->E[j].cmp == -1) && (g->E[j].tgt == g->E[i].src) && (i != j))
	cmp = j;
    }

    if(cmp != -1) {
      g->E[i].cmp = cmp;
      g->E[cmp].cmp = i;
    }
  }

  return g;
}

void write_graph_to_file(const char* fn, Graph* g) {

  FILE* fp = fopen(fn, "w");

  if (!fp) {
    fprintf(stderr, "Error opening file \"%s\".\n", fn);
    exit(EXIT_FAILURE);
  }

  unsigned int i = 0, j = 0;

  fprintf(fp, "%u\n", g->n);
  fprintf(fp, "%u\n", g->m);
  
  for(i = 0; i < g->n; i++)
    for(j = g->V[i].first; j <= g->V[i].last; j++)
      fprintf(fp, "%u %u\n", g->E[j].src, g->E[j].tgt);

  fclose(fp);
}

void free_graph(Graph* g) {
  free(g->V);
  free(g->E);
  free(g);
}
