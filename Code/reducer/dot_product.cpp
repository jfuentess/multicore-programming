/* Compile: g++ -o dot dot_product.cpp -fcilkplus -lcilkrts -lrt */
using namespace std;

#include <iostream>

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

#include <math.h>

#define num_threads __cilkrts_get_nworkers()

/* Sequential dot product */
ulong dot_product_seq(int *X, int *Y, int n) {
  
  ulong global_out = 0;
  
  for(int i = 0; i < n; i++) 
    global_out += X[i]*Y[i];
  
  return global_out;
}

/* Parallel dot product processing block by block */
ulong dot_product_block(int *X, int *Y, int n) {
  int chunk = ceil(n/num_threads);
  ulong *local_outs = new ulong[num_threads];
  ulong global_out = 0;
  
  cilk_for(int i = 0; i < num_threads; i++) {
    int ll = i*chunk;
    int ul = ll + chunk;

    if(ul > n)
      ul = n;
    
    ulong local_out = 0;
    
    for(int j = ll; j < ul; j++)
      local_out += X[j]*Y[j];
    
    local_outs[i];
  }
  
  for(int i = 0; i < num_threads; i++) {
    global_out += local_outs[i];
  }
  
  return global_out;
}

/* Parallel dot product using reducers */
ulong dot_product_reducer(int *X, int *Y, int n) {
  cilk::reducer_opadd<ulong> global_out(0);
  
  cilk_for(int i = 0; i < n; i++) {
    global_out += X[i]*Y[i];
  }
  
  return global_out.get_value();
}

/* Parallel dot product processing the arrays as a tree. It stops when the threshold is reached */
ulong dot_product_tree(int *X, int *Y, int l, int r, int threshold) {
  if(r-l < threshold) {
    int local_out = 0;
    for(int i = l; i <= r; i++)
      local_out += X[i]*Y[i];
    return local_out;
  }

  int m = (l+r)/2;
  
  ulong result1 = cilk_spawn dot_product_tree(X, Y, l, m, threshold);
  ulong result2 = dot_product_tree(X, Y, m+1, r, threshold);
  cilk_sync;
  
  return result1 + result2;
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    cerr << "Usage: " << argv[0] << "<number of elements>" << endl;
    return EXIT_FAILURE;
  }
  int n = atoi(argv[1]);
  int *X = new int[n];
  int *Y = new int[n];

  for(int i = 0; i < n; i++) {
    X[i] = 1;
    Y[i] = 1;
  }
  
  struct timespec stime, etime;
  
  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime);
  ulong out_seq = dot_product_seq(X, Y, n);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime);
  double t1 = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime);
  ulong out_block = dot_product_block(X, Y, n);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime);
  double t2 = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime);
  ulong out_reducer = dot_product_reducer(X, Y, n);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime);
  double t3 = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime);
  ulong out_tree_1 = dot_product_tree(X, Y, 0, n-1, 1);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime);
  double t4 = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime);
  ulong out_tree_512 = dot_product_tree(X, Y, 0, n-1, 512);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime);
  double t5 = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime);
  ulong out_tree_2048 = dot_product_tree(X, Y, 0, n-1, 2048);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime);
  double t6 = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime);
  ulong out_tree_num_threads = dot_product_tree(X, Y, 0, n-1, (int)(n/num_threads));
  clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime);
  double t7 = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

  cout << num_threads << ",seq," << t1 << "," << n << endl;
  cout << num_threads << ",block," << t2 <<  "," << n << endl;
  cout << num_threads << ",reducer," << t3 <<  "," << n << endl;
  cout << num_threads << ",tree_1," << t4 <<  "," << n << endl;
  cout << num_threads << ",tree_512," << t5 <<  "," << n << endl;
  cout << num_threads << ",tree_2048," << t6 <<  "," << n << endl;
  cout << num_threads << ",tree_var," << t7 <<  "," << n << endl;

  return EXIT_SUCCESS;
}
