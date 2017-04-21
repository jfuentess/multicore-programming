/* Compile: g++ -o add reducer_add.cpp -fcilkplus -lcilkrts -lrt */
using namespace std;

#include <iostream>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

int main()
{
	/* Declaration of a reducer supporting thread-safe addition */
    cilk::reducer_opadd<unsigned long> accum(0);
    cilk_for (int i = 0; i < 1000000; i++) {
        *accum += 1;
    }
    cout << accum.get_value() << endl;
}
