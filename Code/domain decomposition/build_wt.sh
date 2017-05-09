cd bitrank
gcc -O2 -std=gnu89 -c *.c

cd ..

echo "Compiling sequential algorithm"
gcc $opt $def -c wavelet_tree.c -lm
gcc $opt $def wavelet_tree.o bitrank/basic.o bitrank/bit_array.o bitrank/bitrankw32int.o -o wt -lm

echo "Compiling parallel algorithm"
gcc $opt $def -c par_wavelet_tree.c -fcilkplus -lcilkrts -lm
gcc $opt $def par_wavelet_tree.o bitrank/basic.o bitrank/bit_array.o bitrank/bitrankw32int.o -o ddwt -fcilkplus -lcilkrts -lm

echo "Done."
