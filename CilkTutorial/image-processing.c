// Compilar: gcc -std=gnu99 -o image image-processing.c -fcilkplus -lcilkrts

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

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

#define num_threads __cilkrts_get_nworkers();

void print_matrix(int nrow, int ncol, double** M, const char* fn) {
  if(nrow > 1024 || ncol > 1024 ) {
    fprintf(stderr, "Too large to be printed\n");
    return;
  }
  printf("Writing matrix ...");

  FILE *fp = fopen(fn, "w");

  fprintf(fp, "%d\n", nrow);
  fprintf(fp, "%d\n", ncol);
  
  for(int i = 0; i < nrow; i++) {
    for(int j = 0; j < ncol; j++)
      fprintf(fp, "%f ", M[i][j]);
    fprintf(fp, "\n");
  }

  fclose(fp);

  printf(" done.\n");
}

double ** process_image(int nrow, int ncol, double **A, double **B) {
  cilk_for(int i = 0; i < nrow; i++) {
    for(int j = 0; j < ncol; j++) {
      double n = 0, w = 0, s = 0, e = 0;
      if(i >= 1)
	n = A[i-1][j];
      if(i < nrow-1)
	s = A[i+1][j];
      if(j >= 1)
	w = A[i][j-1];
      if(j < ncol-1)
	e = A[i][j+1];

      B[i][j] = (n+w+s+e)/4;
    }
  }
  return B;
}

int main(int argc, char* argv[]) {

  if(argc < 4) {
    fprintf(stderr, "Usage: %s <num rows> <num cols> <output file>\n", argv[0]);
    fprintf(stderr, "Example: ./image 512 512 out_file\n");
    exit(EXIT_FAILURE);
  }

  struct timespec stime, etime;
  double t;

  /* Number of rows and columns */
  int nrow = atoi(argv[1]);
  int ncol = atoi(argv[2]);

  /* Output file. This will be used to plot the matrix A as an image */
  char* out_file = argv[3];
  
  double **A = (double **)malloc(nrow*sizeof(double *));
  double **B = (double **)malloc(nrow*sizeof(double *));
  
  for(int i = 0; i < nrow; i++) {
    A[i] = (double *)malloc(ncol*sizeof(double));
    B[i] = (double *)malloc(ncol*sizeof(double));
  }
  
  time_t tt;
  srand((unsigned) time(&tt));

  /* Filling matrix A with random numbers. To show the matrix as an image, the
     values in A must belong to the range [0,1] */
  for(int i = 0; i < nrow; i++)
    for(int j = 0; j < ncol; j++)
      A[i][j] = (double)rand() / (double)RAND_MAX;

  /* Printing the initial matrix */
  print_matrix(nrow, ncol, A, out_file);

  int choice;
  
  do {
    printf("Select an option:\n");
    printf("\t1. Process image\n");
    printf("\t2. Exit\n");
    printf("\n\toption: ");
    scanf("%d", &choice);
    
    switch (choice) {
    case 1:
      if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
  	fprintf(stderr, "clock_gettime failed");
  	exit(-1);
      }

      /* Parallel processing of matrix A */
      A = process_image(nrow, ncol, A, B);
      
      if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
  	fprintf(stderr, "clock_gettime failed");
  	exit(-1);
      }

      /* Printing the resulting matrix */
      print_matrix(nrow, ncol, A, out_file);

      t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
      printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);

      break;
    case 2:
      return EXIT_SUCCESS;
      break;
      
    default:
      break;
    }
    
  } while(choice != 2);
        
  return EXIT_SUCCESS;
}
