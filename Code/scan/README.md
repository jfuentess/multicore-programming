# Scan pattern

Problems to be solved with the *scan pattern*. You can find four problems

* *Prefix sum*: Computation of the prefix sum operation over an **array** of integers. This operation is also known as accumulative sum. To solve this problem, we present several alternatives implemented in the file [prefix_sum.c](prefix_sum.c). To test of performance of each solution, run the bash script in [run_test_ps.sh](run_test_ps.sh)
* *List ranking*: Computation of the prefix sum operation over a **list** of integers. This problem is a generalization of the prefix sum problem. To solve this problem, we present a solution based on the parallel algorithm of [Bader and Cong](https://pdfs.semanticscholar.org/54fb/843f085c978e45868bfabbd819096a060334.pdf). To test of performance of the solution, run the bash script in [run_test_lr.sh](run_test_lr.sh). In order to test our solution, we provide a generator a linked lists, [generate_list.c](generate_list.c)
* *Filtering*: Computation of all of the elements of an array which meet a given condition. In this problem, we apply the **pack** pattern
* *Graph to tree*: Given a graph and a list of edges of the graph, compute the induced tree in parallel.
* *Euler Tour*: Computation of the Euler Tour of a tree.

The details of each problem are available [here](problems.pdf)