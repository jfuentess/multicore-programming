/* Compile: gcc -std=gnu99 -o gl generate_list.c */

/* Generation of linked lists. This code generates a circular linked list with a
   minimum jump */

#include <stdio.h>
#include <stdlib.h>

typedef struct _node_t Node;

struct _node_t {
  uint value;
  int next;
};

Node* generate_list(uint n, uint jump) {
  Node* list = (Node*)malloc(n*sizeof(Node));
  uint global_cnt = 0;
  for(uint i = 0; i < n; i++) {
    list[i].next = -1;
    list[i].value = 1;
  }

  uint curr = 0;
  while(global_cnt < n-1) {
    uint cnt = 0;
    uint idx = curr;

    while(cnt < jump) {
      idx = (idx+1)%n;

      if(list[idx].next == -1 && idx != curr) 
  	cnt++;
    }

    list[curr].next = idx;
    curr = idx;
    global_cnt++;
  }

  for(uint i = 0; i < n; i++) {
    if(list[i].next == -1) {
      list[i].next = 0;
      break;
    }
  }
  return list;
}
  
Node* write_list(Node* list, uint n, const char* fn) {
  FILE* fp = fopen(fn, "w");

  if (!fp) {
    fprintf(stderr, "Error opening file \"%s\".\n", fn);
    exit(EXIT_FAILURE);
  }

  fprintf(fp, "%u\n", n);

  for(uint i=0; i<n; i++)
    fprintf(fp, "%u %d\n", list[i].value, list[i].next);

  fclose(fp);
}
  

int main(int argc, char* argv[]) {
  
  if(argc < 4) {
    fprintf(stderr, "Usage: %s <length of the list> <jump> <output file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  struct timespec stime, etime;
  double t;

  uint n = atoi(argv[1]);
  uint jump = atoi(argv[2]);

  Node* list = generate_list(n, jump);
  write_list(list, n, argv[3]);
 
  return EXIT_SUCCESS;
}
