# Adding 1

This example presents the problem of adding *1* *n* times. This simple example helps us to understand some classical mistakes in multicore programming. The description of the files follows:

* *seq.c*: The sequential implementation to solve the problem
* *par1.c*: First parallel implementation to solve the problem. This solution has a **race condition**, therefore, its result is non-deterministic.
* *par2.c*: Second parallel implementation to solve the problem. This solution is correct, however, its scalability is poor because the implementation has **false sharing**
* *par3.c*: Third parallel implementation to solve the problem. This solution is correct and has good scalability
