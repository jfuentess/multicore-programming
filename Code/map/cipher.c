// Compilar: gcc -std=gnu99 -o cipher cipher.c

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

char* read_sequence(const char* fn, unsigned int* n) {

  FILE* fp = fopen(fn, "r");
  size_t read;

  if (!fp) {
    fprintf(stderr, "Error opening file \"%s\".\n", fn);
    exit(-1);
  }

  fseek(fp, 0L, SEEK_END);
  *n = (unsigned int)ftell(fp);

  char* t;
  t = malloc(*n);

  fseek(fp, 0L, SEEK_SET);

  read = fread(t, sizeof(char), *n, fp);
  if(read != *n){
    fprintf(stderr, "Error reading file \"%s\".\n", fn);
    exit(-1);
  }

  fclose(fp);

  return t;
}

int get_index(char c) {
  int x = 0;
  if(c == 'A')
    x = 10;
  else if(c == 'B')
    x = 11;
  else if(c == 'C')
    x = 12;
  else if(c == 'D')
    x = 13;
  else if(c == 'E')
    x = 14;
  else if(c == 'F')
    x = 15;
  else
    x = c-'0';
  return x;
}

void cipher(char* sequence, char* reflector, unsigned int n) {
  for(int i = 0; i < n; i++)
    sequence[i] = reflector[get_index(sequence[i])];
}

int main(int argc, char* argv[]) {

  if(argc != 2){
    fprintf(stderr, "Execute: %s <input file>\n", argv[0]);
    exit(-1);
  }

  char reflector[10] = "0123456789";
  unsigned int n = 0;
  char* sequence = read_sequence(argv[1], &n);

  cipher(sequence, reflector, n);
  
  printf("Encrypted sequence: %s\n", sequence);
  
  return EXIT_SUCCESS;
}
