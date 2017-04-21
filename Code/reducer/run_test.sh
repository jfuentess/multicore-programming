#!/bin/bash

echo "threads,algo,time,n"

iterations=3

for n in 1000000 10000000 100000000 500000000 # iterate over cores
do
    for (( i=0; i<${iterations}; i++)) # repetitions
    do
	for j in 1 2 4 6 8 10 12 # iterate over cores
	do
	    CILK_NWORKERS=${j} ./dot ${n}
	done
    done
done
