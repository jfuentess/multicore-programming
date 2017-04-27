#!/bin/bash

outfile=results$(date +"%Y-%m-%d-%H:%M:%S").data

echo "threads,algo,time,n,instructions,LLC-load-misses,LLC-stores-misses,cache-misses" >> ${outfile}

iterations=5
algos=("ps_seq" "ps_block" "ps_tree" "ps_iter")
numalgos=${#algos[@]}

for n in 16777216 33554432 67108864 134217728 # iterate over n
do
    for (( a=0; a<${numalgos}; a++)) # repetitions
    do
	for (( i=0; i<${iterations}; i++)) # repetitions
	do
	    for j in 1 2 4 6 8 10 12 # iterate over cores
	    do
		CILK_NWORKERS=${j} perf stat -o perf.tmp -x, -e \
		instructions,LLC-load-misses,LLC-stores-misses,cache-misses \
		./${algos[a]} ${n} >> ${outfile}

		cut -d, -f1 perf.tmp | sed '/#/d' | sed '/^$/d' \
                    | paste -s | sed 's/\s\+/,/g' >> ${outfile}

		echo -e '$-1s/\\n/,/\nx' | ex ${outfile}
	    done
	done
    done
    >&2 echo "Done with n:"${n}
done


rm perf.tmp
