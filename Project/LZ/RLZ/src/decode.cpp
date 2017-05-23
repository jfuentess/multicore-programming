#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "dictionary.h"
#include "common.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: decode [DICTIONARY] [INPUT] > OUTPUT\n");

        exit(EXIT_FAILURE);
    }

    Dictionary d(argv[1]);

    Reader r(argv[2]);

    size_t pos_bits = r.read(8);
    size_t len_bits = r.read(8);
    size_t n = r.read(64);
    size_t i = 0;

    while (i < n) {
        size_t pos = r.read(pos_bits);
        size_t len = r.read(len_bits);
        d.substr(pos, len, stdout);
        i += (len > 0) ? len : 1;
    }

    return EXIT_SUCCESS;
}
