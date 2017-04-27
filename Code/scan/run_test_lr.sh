#!/bin/bash

sleep 10

outfile=results$(date +"%Y-%m-%d-%H:%M:%S").data

echo "threads,algo,time,n,file,instructions,LLC-load-misses,LLC-stores-misses,cache-misses" >> ${outfile}

iterations=5

algos=("par_lr" "seq_lr")

numalgos=${#algos[@]}

datasets=("list.10M.1" "list.10M.8" "list.10M.256" "list.10M.1024"
	  "list.20M.1" "list.20M.8" "list.20M.256" "list.20M.1024"
	  "list.30M.1" "list.30M.8" "list.30M.256" "list.30M.1024"
	  "list.100M.1" "list.100M.8" "list.100M.256")

numdatasets=${#datasets[@]}

for (( d=0; d<${numdatasets}; d++))  # iterate over n
do
    for j in 1 2 4 6 8 10 12 # iterate over cores
    do
	for (( i=0; i<${iterations}; i++)) # repetitions
	do
	    for (( a=0; a<${numalgos}; a++)) # Algorithms
	    do
		CILK_NWORKERS=${j} perf stat -o perf.tmp -x, -e \
			     instructions,LLC-load-misses,LLC-stores-misses,cache-misses \
			     ./${algos[a]} ${datasets[d]} >> ${outfile}
		
		cut -d, -f1 perf.tmp | sed '/#/d' | sed '/^$/d' \
                    | paste -s | sed 's/\s\+/,/g' >> ${outfile}
		
		echo -e '$-1s/\\n/,/\nx' | ex ${outfile}
	    done
	done
    done
done


rm perf.tmp
