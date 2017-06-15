/* Compile: g++ -o queue_boost lockfree-queue.cpp -lboost_system -fcilkplus -lcilkrts 
 *
 * Dependences:
 *   - Boost library:  sudo aptitude install libboost-all-dev or
 *                     download boost library from http://www.boost.org/
 *
 * More documentation about lockfree data structures in Boost library at http://www.boost.org/doc/libs/1_64_0/doc/html/lockfree.html
 *
 */
#include <boost/lockfree/queue.hpp>
#include <iostream>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

boost::lockfree::queue<int> queue(128);

using namespace std;

int main(int argc, char* argv[])
{
  int ops = atoi(argv[1]);
  
  cilk_for (int i = 0; i < ops; ++i)
    queue.push(i);

  int ret;
  cilk_for (int i = 0; i < ops; ++i)
    queue.pop(ret);

  int size = 0;
  while (queue.pop(ret)) size++;

  cout << "[QUEUE] ";
  cout << "There are " << size << " elements after " << ops << " insertions";
  cout << " and " << ops << " deletions" << endl;

  cout << "Using " << __cilkrts_get_nworkers() << " threads" << endl;
}
