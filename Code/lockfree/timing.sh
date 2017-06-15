#!/bin/bash

echo "algo,threads,time,operations,throughput"

SeqAlgos=("lockfree_stack/timing_seq")
ParAlgos=("lockfree_stack/timing_lf" "lockbased_stack/timing_lb")

numseqalgos=${#SeqAlgos[@]}
numparalgos=${#ParAlgos[@]}

iterations=3

for (( k=0; k<${numparalgos}; k++)) # iterate over algorithms
do
    for ops in 1000000 5000000 10000000 50000000 100000000 # iterate over cores
    do
	for (( i=0; i<${iterations}; i++)) # repetitions
	do
	    for j in 1 2 4 6 8 10 12 # iterate over cores
	    do
		CILK_NWORKERS=${j} ./${ParAlgos[k]} ${ops}
	    done
	done
    done
done

for (( k=0; k<${numseqalgos}; k++)) # iterate over algorithms
do
    for ops in 1000000 5000000 10000000 50000000 100000000 # iterate over cores
    do
	for (( i=0; i<${iterations}; i++)) # repetitions
	do
	    ./${SeqAlgos[k]} ${ops}
	done
    done
done
