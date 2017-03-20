/*
 * MatrixMultiplicationSubmatrix_dinamic.c
 *
 *  Created on: March 20, 2017
 *  Author: José Fuentes
 *  Page 50, What Every Programmer Should Know About Memory. Ulrich Drepper. URL: https://www.akkadia.org/drepper/cpumemory.pdf
 *
 * Compile: gcc -o mm MatrixMultiplicationSubmatrix.c
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <time.h>

/* Matrix size */
#define N 1000
/* Cache line size */
#define CLS 64

#define SM (CLS / sizeof(double))

double** create_matrix() {
  int i = 0;
  double** matrix = (double**)calloc(N, sizeof(double*));
  
  for(i = 0; i < N; ++i)
    matrix[i] = (double*)calloc(N, sizeof(double));

  return matrix;
}

double** fill_matrix(double** matrix) {
  int i = 0, j = 0;
  
  for(i = 0; i < N; ++i) 
    for(j = 0; j < N; ++j)
      matrix[i][j] = i*10 + j;

  return matrix;
}

void print_matrix(double** matrix) {
  int i = 0, j = 0;
  
  for(i = 0; i < N; ++i) {
    for(j = 0; j < N; ++j)
      printf("%f\t",matrix[i][j]);

    printf("\n");
  }

  return;
}

int main(int argc, char** argv) {

  double** mul1 = create_matrix();
  mul1 = fill_matrix(mul1);
  double** mul2 = create_matrix();
  mul2 = fill_matrix(mul2);
  double** res = create_matrix();
  
  int i = 0, j = 0, k = 0;
  int i2 = 0, j2 = 0, k2 = 0;

  double* rmul1;
  double* rmul2;
  double* rres;

  struct timespec stime, etime;
  double t;

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  for(i = 0; i < N; i += SM)
    for(j = 0; j < N; j += SM)
      for(k = 0; k < N; k += SM)
	for(i2 = 0, rres = &res[i][j], rmul1 = &mul1[i][k]; i2 < SM; ++i2, rres = &res[i+i2][j], rmul1 = &mul1[i+i2][k])
	  for(k2 = 0, rmul2 = &mul2[k][j]; k2 < SM; ++k2, rmul2 = &mul2[k+k2][j])
	    for(j2 = 0; j2 < SM; ++j2)
	      rres[j2] += rmul1[k2] * rmul2[j2];
	  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  printf("%f secs\n", t); // time in secs

  return EXIT_SUCCESS;
}
