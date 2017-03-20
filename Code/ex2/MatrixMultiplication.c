/*
 * MatrixMultiplication.c
 *
 *  Created on: March 20, 2017
 *  Author: José Fuentes
 *  Page 49, What Every Programmer Should Know About Memory. Ulrich Drepper. URL: https://www.akkadia.org/drepper/cpumemory.pdf
 *
 * Compile: gcc -o mm MatrixMultiplication.c
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <time.h>

#define N 1000

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

  struct timespec stime, etime;
  double t;

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  for(i = 0; i < N; ++i)
    for(j = 0; j < N; ++j)
      for(k = 0; k < N; ++k)
	res[i][j] += mul1[i][k] * mul2[k][j];

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  printf("%f secs\n", t); // time in secs

  return EXIT_SUCCESS;
}
