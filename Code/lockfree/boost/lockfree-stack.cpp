/* Compile: g++ -o stack_boost lockfree-stack.cpp -lboost_system -fcilkplus -lcilkrts 
 *
 * Dependences:
 *   - Boost library:  sudo aptitude install libboost-all-dev or
 *                     download boost library from http://www.boost.org/
 *
 * More documentation about lockfree data structures in Boost library at http://www.boost.org/doc/libs/1_64_0/doc/html/lockfree.html
 *
 */
#include <boost/lockfree/stack.hpp>
#include <iostream>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

boost::lockfree::stack<int> stack(128);

using namespace std;

int main(int argc, char* argv[])
{
  int ops = atoi(argv[1]);
  
  cilk_for (int i = 0; i < ops; ++i)
    stack.push(i);

  int ret;
  cilk_for (int i = 0; i < ops; ++i)
    stack.pop(ret);

  int size = 0;
  while (stack.pop(ret)) size++;

  cout << "[STACK] ";
  cout << "There are " << size << " elements after " << ops << " insertions";
  cout << " and " << ops << " deletions" << endl;

  cout << "Using " << __cilkrts_get_nworkers() << " threads" << endl;
}
