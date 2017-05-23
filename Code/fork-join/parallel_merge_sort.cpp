// Compilar: g++ -std=c++11 -o pms parallel_merge_sort.cpp -fcilkplus -lcilkrts
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
 * + y = std::move(X) is similar to y=x, except that it gives license to change
 *   the value of x. For example, suppose y is an empty vector and x is a vector
 *   with N elements. y=std::move(x) can be done in O(1) time, because it only
 *   needs to move internal pointers and size information fron x to y, and not
 *   copy the vector's elements.
 *
 * + std::move(first, last, result): Moves items in the range [first,last) to
 *   [result,result+(last-first)). Afterward, items in the range [first,last) have
 *   unspecified values.
 *
 * + std::upper_bound(first, last, val): Returns an iterator pointing to the
 *   first element in the range [first,last) which compares greater than val. 
 *
 * + std::lower_bound(first, last, val): Returns an iterator pointing to the
 *   first element in the range [first,last) which does not compare less than val. 
 *
 * + std::stable_sort(first, last): Sorts the elements in the range [first,last)
 *   into ascending order, like sort, but stable_sort preserves the relative
 *   order of the elements with equivalent values. 
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



void serial_merge( int* xs, int* xe, int* ys, int* ye, int* zs ) {
    while( xs!=xe && ys!=ye ) {
        bool which = *ys < *xs;
        *zs++ = std::move(which ? *ys++ : *xs++);
    }
    std::move( xs, xe, zs );
    std::move( ys, ye, zs );
}

void parallel_merge( int* xs, int* xe, int* ys, int* ye, int* zs ) {
    const size_t MERGE_CUT_OFF = 2000;
    if( xe-xs + ye-ys <= MERGE_CUT_OFF ) {
        serial_merge(xs,xe,ys,ye,zs);
    } else {
        int *xm, *ym;
        if( xe-xs < ye-ys  ) {
            ym = ys+(ye-ys)/2;
            xm = std::upper_bound(xs,xe,*ym);
        } else {
            xm = xs+(xe-xs)/2;
            ym = std::lower_bound(ys,ye,*xm);
        }
        int* zm = zs + (xm-xs) + (ym-ys);
        cilk_spawn parallel_merge( xs, xm, ys, ym, zs );
        /*nospawn*/parallel_merge( xm, xe, ym, ye, zm );
        // implicit cilk_sync
    }
}

void parallel_merge_sort( int* xs, int* xe, int* zs, bool inplace ) {
    const size_t SORT_CUT_OFF = 500;
    if( xe-xs<=SORT_CUT_OFF ) {
       std::stable_sort( xs, xe );
       if( !inplace ) 
           std::move( xs, xe, zs );
   } else {
       int* xm = xs + (xe-xs)/2;
       int* zm = zs + (xm-xs);
       int* ze = zs + (xe-xs);
       cilk_spawn parallel_merge_sort( xs, xm, zs, !inplace );
       /*nospawn*/parallel_merge_sort( xm, xe, zm, !inplace );
       cilk_sync;
       if( inplace )
           parallel_merge( zs, zm, zm, ze, xs );
       else
           parallel_merge( xs, xm, xm, xe, zs );
   }
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

  if(argc < 2) {
    fprintf(stderr, "Usage: %s <number of elements>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int n = atoi(argv[1]);
  int *x = generate_array(n);
  int *buff = (int *)malloc(n*sizeof(int));
  
  struct timespec stime, etime;
  double t;
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  parallel_merge_sort( x, x+n, buff, false );

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);
  
  return EXIT_SUCCESS;
}
