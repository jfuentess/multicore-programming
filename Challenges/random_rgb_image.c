/* Compilar: gcc -o random_rgb random_rgb_image.c
 *
 * Generate a random RGB image in PPM ASCII format
 */

#include <stdio.h>
#include <stdlib.h>

void generate_image(int nrow, int ncol, const char* fn) {
  FILE *fp = fopen(fn, "w");

  fprintf(fp, "P3\n");
  fprintf(fp, "%d\n", nrow);
  fprintf(fp, "%d\n", ncol);
  fprintf(fp, "255\n");
  
  for(int i = 0; i < nrow; i++) {
    for(int j = 0; j < ncol; j++) {
      for(int k = 0; k < 3; k++)
	fprintf(fp, "%d ", rand()%256);
      fprintf(fp, "\n");
    }
  }

  fclose(fp);
}

int main(int argc, char* argv[]) {

  if(argc < 4) {
    fprintf(stderr, "Usage: %s <num rows> <num cols> <output file>\n", argv[0]);
    fprintf(stderr, "Example: ./image 512 512 out_file\n");
    exit(EXIT_FAILURE);
  }

  /* Number of rows and columns */
  int nrow = atoi(argv[1]);
  int ncol = atoi(argv[2]);

  /* Output file*/
  char* out_file = argv[3];
  
  generate_image(nrow, ncol, out_file);
  
  return EXIT_SUCCESS;
}
