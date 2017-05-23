# Fork-join pattern

Algorithms to solve the sorting problem with the *fork-join pattern*. You can
find four algorithms

* *Sequential*: Sequential implementation to solve the sorting problem. This
   implementation uses the **stable_sort** function of the STL of C++. The
   implementation is available at [sequential_sort.cpp](sequential_sort.cpp)

* *Parallel merge sort*: Parallel version of the merge sort algorithm. This
   implementation includes a parallel version for the merge step. The
   implementation is available at
   [parallel_merge_sort.cpp](parallel_merge_sort.cpp)

* *Parallel quick sort*: Parallel version of the quick sort algorithm. The
   implementation is available at
   [parallel_quick_sort.cpp](parallel_quick_sort.cpp)

* *Parallel counting sort*: Available soon

The details of the problem and some links are available [here](problems.pdf)