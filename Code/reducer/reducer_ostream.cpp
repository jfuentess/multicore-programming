/* Compile: g++ -o ostream reducer_ostream.cpp -fcilkplus -lcilkrts */
using namespace std;

#include <iostream>
#include <string>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_ostream.h>

void reducer_ostream(int n) {
  cout << "Standard cout: ";
  cilk_for(int ch = 'a'; ch <= 'z'; ch++) {
    cilk_for(int i = 0; i < n; i++) {
      char buf[4] = {'\0', '\0'};
      buf[0] = ch;
      cout << buf;
    }
  }
  cout << endl;
  
  /* Reducer to support thread-safe printing in a stream. In this example, thread-safe printing in the standard output cout */
  cilk::reducer_ostream hyper_cout(cout);
  *hyper_cout << "Reducer cout:  ";
  cilk_for(int ch = 'a'; ch <= 'z'; ch++) {
    cilk_for(int i = 0; i < n; i++) {
      char buf[4] = {'\0', '\0'};
      buf[0] = ch;
      *hyper_cout << buf;
    }
  }
  *hyper_cout << endl;
}

int main(int argc, char **argv)
{
  
  if(argc != 2) {
    cerr << "Usage: " << argv[0] << " <number of repetitions>" << endl;
    return EXIT_FAILURE;
  }
  
  int n = atoi(argv[1]);

  reducer_ostream(n);
  return 0;
}
