/* Note by Jose Fuentes: This code is a edited version of the code available at
   https://gist.github.com/andrejbauer/7919569 */
/*
  This program is an adaptation of the Mandelbrot program
  from the Programming Rosetta Stone, see
  http://rosettacode.org/wiki/Mandelbrot_set

  Compile the program with:

  gcc -std=gnu99 -o mandelbrot mandelbrot.c -fcilkplus -lcilkrts

  Usage:
 
  ./mandelbrot <xmin> <xmax> <ymin> <ymax> <maxiter> <xres> <out.ppm>

  Example:

  ./mandelbrot 0.27085 0.27100 0.004640 0.004810 1000 1024 pic.ppm

  The interior of Mandelbrot set is black, the levels are gray.
  If you have very many levels, the picture is likely going to be quite
  dark. You can postprocess it to fix the palette. For instance,
  with ImageMagick you can do (assuming the picture was saved to pic.ppm):

  convert -normalize pic.ppm pic.png

  The resulting pic.png is still gray, but the levels will be nicer. You
  can also add colors, for instance:

  convert -negate -normalize -fill blue -tint 100 pic.ppm pic.png

  See http://www.imagemagick.org/Usage/color_mods/ for what ImageMagick
  can do. It can do a lot.
*/

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>
#include <complex.h>

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

void print_mandelbrot(int **A, int xres, int yres, double xmin, double xmax,
		      double ymin, double ymax, uint16_t
		      maxiter, double dy, double dx, const char*
  fn) {    
  /* Open the file and write the header. */
  FILE * fp = fopen(fn,"wb");
  char *comment="# Mandelbrot set";/* comment should start with # */

  /*write ASCII header to the file*/
  fprintf(fp,
          "P6\n# Mandelbrot, xmin=%lf, xmax=%lf, ymin=%lf, ymax=%lf, maxiter=%d\n%d\n%d\n%d\n",
          xmin, xmax, ymin, ymax, maxiter, xres, yres, (maxiter < 256 ? 256 : maxiter));

  for(int j = 0; j < yres; j++) {
    for(int i = 0; i < xres; i++) {

      if (A[i][j] >= maxiter) {
	/* interior */
	const unsigned char black[] = {0, 0, 0, 0, 0, 0};
	fwrite (black, 6, 1, fp);
      }
      else {
	/* exterior */
	unsigned char color[6];
	color[0] = A[i][j] >> 8;
	color[1] = A[i][j] & 255;
	color[2] = A[i][j] >> 8;
	color[3] = A[i][j] & 255;
	color[4] = A[i][j] >> 8;
	color[5] = A[i][j] & 255;
	fwrite(color, 6, 1, fp);
      }
    }
  }
    fclose(fp);
}

int mandelbrot(int **A, int xres, int yres, double xmin, double ymax, uint16_t
	   maxiter, double dy, double dx) {

//#pragma cilk grainsize = 1
  cilk_for(int i = 0; i < xres; i++) {
    cilk_for (int j = 0; j < yres; j++) {
      double y = ymax - j * dy;
      double u = 0.0;
      double v= 0.0;
      double u2 = u * u;
      double v2 = v*v;
      double x = xmin + i * dx;
      /* iterate the point */
      int counter=0;
      for (int k = 1; k < maxiter && (u2 + v2 < 4.0); k++) {
            v = 2 * u * v + y;
            u = u2 - v2 + x;
            u2 = u * u;
            v2 = v * v;
	    counter++;
      }

      A[i][j] = counter;
    }
  }
  
}

int main(int argc, char* argv[]) {

  /* Parse the command line arguments. */
  if (argc != 8) {
    printf("Usage:   %s <xmin> <xmax> <ymin> <ymax> <maxiter> <xres> <out.ppm>\n", argv[0]);
    printf("Example: %s 0.27085 0.27100 0.004640 0.004810 1000 1024 pic.ppm\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  struct timespec stime, etime;
  double t;
  
  /* The window in the plane. */
  const double xmin = atof(argv[1]);
  const double xmax = atof(argv[2]);
  const double ymin = atof(argv[3]);
  const double ymax = atof(argv[4]);

  /* Maximum number of iterations, at most 65535. */
  const uint16_t maxiter = (unsigned short)atoi(argv[5]);

  /* Image size, width is given, height is computed. */
  const int xres = atoi(argv[6]);
  const int yres = (xres*(ymax-ymin))/(xmax-xmin);

  /* Precompute pixel width and height. */
  double dx=(xmax-xmin)/xres;
  double dy=(ymax-ymin)/yres;
  
  int **A = (int **)malloc(xres*sizeof(int *));
  
  for(int i = 0; i < xres; i++)
    A[i] = (int *)malloc(yres*sizeof(int));
  
  
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }

  mandelbrot(A, xres, yres, xmin, ymax, maxiter, dy, dx);

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
    fprintf(stderr, "clock_gettime failed");
    exit(-1);
  }
  
  // print_mandelbrot(A, xres, yres, xmin, xmax, ymin, ymax, maxiter, dy, dx, argv[7]);

  t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;
  
  printf("threads: %d, elapsed time: %lf\n", __cilkrts_get_nworkers(), t);
  
  return EXIT_SUCCESS;
}
