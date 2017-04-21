/* Compile: g++ -o min reducer_min.cpp -fcilkplus -lcilkrts -lrt */
using namespace std;

#include <stdio.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_min.h>


void reducer_min_test(uint* A, int n)
{
  /* Reducer to compute the minimum value of an array */
  cilk::reducer_min<uint> min;  
  cilk_for(unsigned i = 0; i < n; ++i) {
    min->calc_min(A[i]);
  }
  
  printf("Minimum value = %u\n", min->get_value());
}

int main(int argc, char* argv[])
{
  if(argc != 2) {
    fprintf(stderr, "Usage: %s <number of elements>\n", argv[0]);
    return EXIT_FAILURE;
  }
  int n = atoi(argv[1]);
  uint *A = new uint[n];

  for(int i=0; i<n; i++)
    A[i] = i+1;

  reducer_min_test(A, n);
  return 0;
}
