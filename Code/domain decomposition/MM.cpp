/* Compile: g++ strassen.cpp common.c -o strassen -fcilkplus -lcilkrts
 *
 * Reference: Parallel algorithm implementing Strassen’s Algorithm for
 * matrix-matrix multiplication (Bradley Kuszmaul). URL: https://software.intel.com/en-us/courseware/256196 
 *
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>  
#include <stdlib.h> 
#include <string.h>

#include <sys/time.h>
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

#define num_threads __cilkrts_get_nworkers()

#include "mm.h"

const long long GRAIN = 2048;

/* =========== Triply-Nested Loop implementation ============ */
void seqMatMult(int m, int n, int p, const double* A, const double* B, double* C)  
{   
	cilk_for (int i = 0; i < m; i++)  
		cilk_for (int j = 0; j < n; j++)   
		{         
			C[i*n+j] = 0.0;  
			for (int k = 0; k < p; k++)  
				C[i*n+j] += A[i*p+k]*B[k*n+j];  
		}  
} 

void my_dgemm_into (int m, int n, int p, const double *A, int AN, const double *B, int BN, double *C, int CN, double beta) {
    for (int i=0; i<m; i++)
	for (int j=0; j<n; j++) {
	    double sum = 0;
	    for (int k=0; k<p; k++)
		sum += A[i*AN + k]*B[k*BN + j];
	    double *CP = &C[i*CN + j];
	    *CP = (*CP)*beta + sum;
	}
}

/* =========== Divide and Conquer Implementation: Using C-style 1-D arrays ============ */
void DCMM_leaf_into (int m, int n, int p, const double *A, const double *B,
		     double *C, int N, int P) {
	my_dgemm_into(m, n, p, A, P, B, N, C, N, 1.0);
}

void DCMM (int m, int n, int p,
      const double *A, // m by p
      const double *B, // p by n
            double *C, // m by n
      int N, // row length for B and C
      int P  // row length for A
      )
{
	if (m==1 || n==1 || p==1 ||
	    (long long)m * (long long)n * (long long)p < GRAIN) {
		DCMM_leaf_into(m, n, p, A, B, C, N, P);
	} else {
		int m2 = m/2;
		int n2 = n/2;
		int p2 = p/2;
		cilk_spawn DCMM(m2  , n2  , p2  , A        , B        , C        , N, P);
		cilk_spawn DCMM(m2  , n-n2, p2  , A        , B     +n2, C+n2     , N, P);
		cilk_spawn DCMM(m-m2, n2  , p2  , A   +m2*P, B        , C   +m2*N, N, P);
		cilk_spawn DCMM(m-m2, n-n2, p2  , A   +m2*P, B     +n2, C+n2+m2*N, N, P);
		cilk_sync;
		cilk_spawn DCMM(m2  , n2  , p-p2, A+p2     , B+p2*N   , C        , N, P);
		cilk_spawn DCMM(m2  , n-n2, p-p2, A+p2     , B+p2*N+n2, C+n2     , N, P);
		cilk_spawn DCMM(m-m2, n2  , p-p2, A+p2+m2*P, B+p2*N   , C   +m2*N, N, P);
		cilk_spawn DCMM(m-m2, n-n2, p-p2, A+p2+m2*P, B+p2*N+n2, C+n2+m2*N, N, P);
	}
}

void DivideAndConquerMM(int m, int n, int p, const double* A, const double* B, double* C)  
{   
	for (int i=0; i<m*n; i++) C[i]=0;
	DCMM(m, n, p, A, B, C, n, p);
} 
 
void
measure(void (*mm)(int m, int n, int p, const double* A, const double* B, double* C),
	int m, int n, int p, const double* A, const double* B, double *C,
	const char *descr)
{
	printf("Executing %-40s", descr);
	fflush(stdout);
	struct timeval before,after;
	gettimeofday(&before, NULL);
	mm(m, n, p, A, B, C);
	gettimeofday(&after,  NULL);
	double tdiff = after.tv_sec - before.tv_sec + (1e-6)*(after.tv_usec - before.tv_usec);
	printf("  Done in %11.6f secs (%u threads)\n", tdiff, num_threads);

}

int main(int argc, char* argv[]) {
  int do_slow = 1;
  while (argc>=2) {
    if (0==strcmp(argv[1], "--fast")) {
      do_slow = 0;
    } else {
      break;
    }
    argv++;
    argc--;
  }
  
  assert(argc==4);
  int M = parse_pos_int(argv[1]);
  int N = parse_pos_int(argv[2]);
  int P = parse_pos_int(argv[3]);
  
  const double *A, *B;
  double *C;
  random_init(M, N, P, &A, &B, &C);
  
  measure(seqMatMult, M, N, P, A, B, C, "Standard matrix multiply (MM).");
  measure(DivideAndConquerMM,   M, N, P, A, B, C, "Divide and Conquer");
  
  free((void*)A);
  free((void*)B);
  free(C);
  return 0;  
} 
