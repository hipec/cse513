#include<iostream>
#include "timer.h"
#include <future>
using namespace std;

#ifdef ASYNC
#define POLICY std::launch::async
#else
#define POLICY std::launch::deferred
#endif

int fib(int n) {
  if (n < 2) {
    return n;
  } else {
   std::future<int> f1 = std::async (POLICY, [&]() {return fib(n-1);});
   std::future<int> f2 = std::async (POLICY, [&]() {return fib(n-2);});
   return f1.get(); f2.get();
  }
}

int main(int argc, char ** argv) {
  int n = argc>1?atoi(argv[1]):40;
  int res;
  timer::kernel([&]() {
    res = fib(n);
  });
  cout<<"Fib(" <<n<<") = "<<res<<endl;
}
