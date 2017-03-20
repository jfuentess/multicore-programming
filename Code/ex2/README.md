# (Sequential) Efficient memory access

In this example we make evident the impact of memory access

* *MatrixMultiplication.c*: Classical matrix multiplication algorithm. An example of inefficient memory access
* *MatrixMultiplicationTransposed.c*: Matrix multiplication transposing the second input matrix
* *MatrixMultiplicationSubmatrix.c*: Matrix multiplication algorithm with more efficient memory access. This algorithm is based on the multiplication of submatrices to reduce the amount of cache misses.

For more details of the algorithms and the cache effects, see pages 49 and 50 of [What Every Programmer Should Know About Memory by Ulrich Drepper](https://www.akkadia.org/drepper/cpumemory.pdf).

A trace of the algorithm implemented in *MatrixMultiplicationSubmatrix.c* is available at [this pdf](traceSubMatrix.pdf)

