/* Compile: g++ -o dfs dfs_traversal.cpp -fcilkplus -lcilkrts -lrt */
using namespace std;

#include <iostream>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_list.h>

#include <sys/time.h>
#include <time.h>
#include <math.h>

#define num_threads __cilkrts_get_nworkers()

/* Global list (no thread-safe) */
list<int> seq_dfs;
/* Global reducer list (thread-safe) */
cilk::reducer< cilk::op_list_append<int> > par_dfs;

/* Create a binary tree */
int *create_binary_tree(int n) {
  int * T = new int[n];
  
  for(int i=0; i < n; i++)
    T[i] = i;
  
  return T;
}

/* Sequential DFS */
void dfs_seq(int* T, int num_nodes, int curr_node) {
  if(curr_node >= num_nodes)
    return;
  
  seq_dfs.push_back(T[curr_node]);
  
  dfs_seq(T, num_nodes, 2*curr_node+1);
  dfs_seq(T, num_nodes, 2*curr_node+2);
}

/* Parallel DFS using reducers */
void dfs_par(int* T, int num_nodes, int curr_node) {
  if(curr_node >= num_nodes)
    return;
  
  par_dfs->push_back(T[curr_node]);
  
  cilk_spawn dfs_par(T, num_nodes, 2*curr_node+1);
  dfs_par(T, num_nodes, 2*curr_node+2);
  cilk_sync;
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    cerr << "Usage: " << argv[0] << " <number of levels>" << endl;
    return EXIT_FAILURE;
  }
  
  int l = atoi(argv[1]);
  int num_nodes = pow(2,l)-1;
  
  int * T = create_binary_tree(num_nodes);
  
  struct timespec stime, etime;
  double t;
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    cerr << "clock_gettime failed" << endl;
    exit(-1);
  }
  
  dfs_seq(T, num_nodes, 0);
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    cerr << "clock_gettime failed" << endl;
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  cout << "Sequential DFS: " << t << " secs" << endl;
  
  // cout << "Sequential DFS:";
  // for(list<int>::const_iterator i = seq_dfs.begin(); i !=
  // 	seq_dfs.end(); i++) { 
  //   cout << " " << *i;
  // }
  // cout << endl;
  
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    cerr << "clock_gettime failed" << endl;
    exit(-1);
  }
  
  dfs_par(T, num_nodes, 0);
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    cerr << "clock_gettime failed" << endl;
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  cout << "Parallel DFS: " << t << " secs";
  cout << " with " << num_threads << " threads" << endl;

  // const list<int> &par_list = par_dfs.get_value();
  // cout << "Parallel DFS:";
  // for(list<int>::const_iterator i = par_list.begin(); i != par_list.end(); i++) {
  //   cout << " " << *i;
  // }
  // cout << endl;

  return EXIT_SUCCESS;
}
