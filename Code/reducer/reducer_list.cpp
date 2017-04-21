/* Compile: g++ -o list reducer_list.cpp -fcilkplus -lcilkrts */
using namespace std;

#include <iostream>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_list.h>


void reducer_list_test(int n) {
  cilk::reducer< cilk::op_list_append<int> > ints_reducer;
  list<int> ints_normal;
  // Build a list of integers, maintaining the order
  cilk_for(int i = 0; i < n; i++) {
    ints_reducer->push_back(i);
    ints_normal.push_back(i);
  }
  
  // Fetch the result of the reducer as a standard STL list
  const list<int> &ints = ints_reducer.get_value();
  
  // Show the resulting lists
  cout << "Integers from reducer_list:";
  for(list<int>::const_iterator i = ints.begin(); i != ints.end(); i++) {
    cout << " " << *i;
  }
  cout << endl;
  
  cout << "Integers from the normal list:";
  for(list<int>::const_iterator i = ints_normal.begin(); i !=
	ints_normal.end(); i++) { 
    cout << " " << *i;
  }
  cout << endl;
  
  cout << "reducer " << ints.size() << endl;
  cout << "normal " << ints_normal.size() << endl;
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    cerr << "Usage: " << argv[0] << " <number of elements>" << endl;
    return EXIT_FAILURE;
  }
  
  int n = atoi(argv[1]);
  
  reducer_list_test(n);
  
  return EXIT_SUCCESS;
}
