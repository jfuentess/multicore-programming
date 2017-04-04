# Pitfalls in multicore programming

Examples of pitfalls in multicore programming

* *race1.c*: Example of a race condition over a memory location
* *race2.c*: Example of a race condition in the standard output 
* *race3.c*: Alternative to solve the race condition in **race1.c** using atomic operations
* *benignrace.c*: Example of a benign race condition. In this example, we find all the distinct symbols of an array. Assumption: The symbols belongs to an integer and contiguous alphabet 
* *falsesharing.c*: Example of false sharing using an interleaving pattern to access memory
* *falsesharing2.c*: Alternative to solve the false sharing in **falsesharing.c** 
