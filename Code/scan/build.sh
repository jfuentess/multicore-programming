echo "Compiling sequential prefix sum"
gcc -std=gnu99 -DSEQ -o ps_seq prefix_sum.c -fcilkplus -lcilkrts -lm -lrt
echo "Compiling parallel prefix sum (tree)"
gcc -std=gnu99 -DTREE -o ps_tree prefix_sum.c -fcilkplus -lcilkrts -lm -lrt
echo "Compiling parallel prefix sum (iter)"
gcc -std=gnu99 -DITER -o ps_iter prefix_sum.c -fcilkplus -lcilkrts -lm -lrt
echo "Compiling parallel prefix sum (block)"
gcc -std=gnu99 -DBLOCK -o ps_block prefix_sum.c -fcilkplus -lcilkrts -lm -lrt
echo "Compiling sequential list ranking"
gcc -std=gnu99 -DSEQ -o lr_seq list_ranking.c -fcilkplus -lcilkrts -lm -lrt
echo "Compiling parallel list ranking"
gcc -std=gnu99 -DPAR -o lr_par list_ranking.c -fcilkplus -lcilkrts -lm -lrt
echo "Compiling parallel list ranking with debug"
gcc -std=gnu99 -DPAR -DDEBUG -o lr_par_debug list_ranking.c -fcilkplus -lcilkrts -lm -lrt
