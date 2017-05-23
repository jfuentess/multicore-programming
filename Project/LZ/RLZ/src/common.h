#ifndef COMMON_H
#define COMMON_H

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sys/time.h>

struct Reader {
  uint8_t buffer;
  size_t nbits;
  FILE *fp;
  Reader(const char *path) : buffer(0), nbits(0) {
    assert(path);
    fp = fopen(path, "rb");
    assert(fp);
  }
  ~Reader() {
    close();
  }
  void close() {
    if (fp == NULL) return;
    fclose(fp);
    fp = NULL;
  }
  void next() {
    if (1 != fread(&buffer, 1, 1, fp)) {
      printf("ERR");
    }
    nbits = 8;
  }
  size_t read() {
    if (nbits == 0) next();
    size_t bit = buffer & 1;
    buffer >>= 1;
    nbits--;
    return bit;
  }
  size_t read(size_t bits) {
    assert(bits > 0);
    size_t value = 0;
    size_t pos = 0;
    while (pos < bits) {
      value |= (read() << pos);
      pos++;
    }
    return value;
  }
};

struct Writer {
  uint8_t buffer;
  size_t nbits;
  FILE *fp;
  Writer(FILE *stream) : buffer(0), nbits(0), fp(stream) {
    assert(stream);
  }
  Writer(const char *path) : buffer(0), nbits(0) {
    assert(path);
    fp = fopen(path, "wb");
    assert(fp);
  }
  ~Writer() {
    close();
  }
  void close() {
    if (fp == NULL) return;
    flush();
    fclose(fp);
    fp = NULL;
  }
  void flush() {
    if (nbits == 0) return;
    fwrite(&buffer, 1, 1, fp);
    buffer = 0;
    nbits = 0;
  }
  void write(size_t bit) {
    assert(bit < 2);
    if (nbits == 8) flush();
    buffer |= (bit << nbits);
    nbits++;
  }
  void write(size_t value, size_t bits) {
    assert(bits > 0);
    do {
      write(value & 1);
      value >>= 1;
      bits--;
    } while (bits > 0);
  }
};

size_t clog2(size_t v) {
  assert(v > 0);
  return (size_t)ceil(log(v)/log(2));
}

size_t s2b(const char *str)
{
  assert(str);

  char *ptr;

  size_t bytes = strtol(str, &ptr, 10);

  switch(*ptr) {
    case 'g':
    case 'G':
      bytes *= 1000; // Fall through.

    case 'm':
    case 'M':
      bytes *= 1000; // Fall through.

    case 'k':
    case 'K':
      bytes *= 1000; // Fall through.

    case 'b':
    case 'B':
    default:
      break;
  }

  return bytes;
}

uint8_t *load_text(char *path, size_t *n)
{
  FILE *fp = fopen(path, "r");
  assert(fp);

  fseek(fp, 0L, SEEK_END);

  *n = (size_t)ftell(fp);

  fseek(fp, 0L, SEEK_SET);

  uint8_t *text = new uint8_t[(*n) + 1];

  if ((*n) != fread(text, 1, *n, fp)) {
    printf("ERR");
  }

  fclose(fp);

  return text;
}

FILE * open_file(char *path, size_t *n)
{
  FILE *fp = fopen(path, "r");
  assert(fp);

  fseek(fp, 0L, SEEK_END);

  *n = (size_t)ftell(fp);

  fseek(fp, 0L, SEEK_SET);

  return fp;
}

void open_fs(char *path, std::ifstream *fs, size_t *n)
{
  FILE *fp = fopen(path, "r");
  assert(fp);

  fseek(fp, 0L, SEEK_END);

  *n = (size_t)ftell(fp);

  fseek(fp, 0L, SEEK_SET);
  fclose(fp);

  fs->open(path);
  assert(fs->good());
  return;
}

size_t file_size(char *path)
{
  FILE *fp = fopen(path, "r");
  assert(fp);

  fseek(fp, 0L, SEEK_END);

  size_t ans  = (size_t)ftell(fp);

  fseek(fp, 0L, SEEK_SET);
  fclose(fp);

  return ans;
}

long double wclock() {
  timeval tim;
  gettimeofday(&tim, NULL);
  return tim.tv_sec + (tim.tv_usec / 1000000.0L);
}

#endif /* COMMON_H */
