// Compilar: gcc -std=gnu99 -o genchars generate_random_chars.c

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

  if(argc != 2) {
    fprintf(stderr, "Usage: %s <number of chars>\n", argv[0]);
    return EXIT_FAILURE;
  }

  uint num_chars = atoi(argv[1]);
  fprintf(stderr, "num_chars: %u\n", num_chars);
  for(int i=0; i < num_chars; i++ ) {
    char randomletter = 'A' + (random() % 26);
    printf("%c", randomletter);
  }
  return EXIT_SUCCESS;
}
