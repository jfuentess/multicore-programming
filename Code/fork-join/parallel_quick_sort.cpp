// Compilar: g++ -std=c++11 -o pqs parallel_quick_sort.cpp -fcilkplus -lcilkrts
/*
  Original code available at
  http://parallelbook.com/sites/parallelbook.com/files/code20131121.zip
*/
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

#include <algorithm>    // std::lower_bound, std::upper_bound, std::sort

/*
 * STL functions used in this code:
 *
 * + std::sort(first, last): Sorts the elements in the range [first,last) into
 *   ascending order. 
 *
 * + std::find_if(first, last, pred): Returns an iterator to the first element
 *   in the range [first,last) for which pred returns true. If no such element
 *   is found, the function returns last. 
 *
 * + std::swap(a, b): Exchanges the values of a and b.
 *
 * + std::partition(first, last, pred): Rearranges the elements from the range
 *   [first,last), in such a way that all the elements for which pred returns
 *   true precede all those for which it returns false. The iterator returned
 *   points to the first element of the second group. 
 *
 */

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

// Size of parallel base case.
ptrdiff_t QUICKSORT_CUTOFF = 500;

// Choose median of three keys.
int* median_of_three(int* x, int* y, int* z) {
    return *x<*y ? *y<*z ? y : *x<*z ? z : x 
         : *z<*y ? y : *z<*x ? z : x;
}

// Choose a partition key as median of medians.
int* choose_partition_key( int* first, int* last ) {
    size_t offset = (last-first)/8;
    return median_of_three(
        median_of_three(first, first+offset, first+offset*2),
        median_of_three(first+offset*3, first+offset*4, last-(3*offset+1)), 
        median_of_three(last-(2*offset+1), last-(offset+1), last-1 )
    );
}

// Choose a partition key and partition [first...last) with it.
// Returns pointer to where the partition key is in partitioned sequence.
// Returns NULL if all keys in [first...last) are equal.
int* divide( int* first, int* last ) {
    // Move partition key to front.
    std::swap( *first, *choose_partition_key(first,last) );
    // Partition 
    int key = *first;
    int* middle = std::partition( first+1, last, [=](const int& x) {return x<key;} ) - 1;
    if( middle!=first ) {
        // Move partition key to between the partitions
        std::swap( *first, *middle );
    } else {
        // Check if all keys are equal
        if( last==std::find_if( first+1, last, [=](const int& x) {return key<x;} ) )
            return NULL;
    }
    return middle;
}

void parallel_quicksort( int* first, int* last ) {
    if( last-first<=QUICKSORT_CUTOFF ) {
      std::sort(first,last);
    } else {
        // Divide
        if( int* middle = divide(first,last) ) {
            // Conquer subproblems in parallel
            cilk_spawn parallel_quicksort( first, middle );
            parallel_quicksort( middle+1, last );
            // no cilk_sync needed here because of implicit one later
        }
    }
    // Implicit cilk_sync when function returns
}
int *generate_array(int n) {
  int *x = (int *)malloc(n*sizeof(int));
  
  for (int i = 0; i < n; i++)
    x[i] = rand();

  return x;
}

void print_array(int *x, int n) {
  printf("Array : ");
  for (int i = 0; i < n; i++)
    printf("%u ", x[i]);
  printf("\n");
}

int main(int argc, char* argv[]) {

  int n = atoi(argv[1]);
  int *x = generate_array(n);
  int *buff = (int *)malloc(n*sizeof(int));
  
  struct timespec stime, etime;
  double t;
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  parallel_quicksort( x, x+n );

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);
  
  return EXIT_SUCCESS;
}
