#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>


int main(int argc, char* argv[]) {
	int n=atoi(argv[1]);
	int total = 0;
	
	struct timespec stime, etime;
	double t;

	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &stime)) {
		fprintf(stderr, "clock_gettime failed");
		exit(-1);
	}

	for(int i = 0; i < n; i++)
	  total += 1;

	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID , &etime)) {
		fprintf(stderr, "clock_gettime failed");
		exit(-1);
	}

	t = (etime.tv_sec - stime.tv_sec) + (etime.tv_nsec - stime.tv_nsec) / 1000000000.0;

	/* threads, elapsed time, total */
	printf("%d,%lf,%d\n", 1, t, total);


	return EXIT_SUCCESS;
}
