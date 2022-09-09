#include<iostream>
#include<thread>
#include "timer.h"
using namespace std;

int fib(int n) {
  if (n < 2) {
    return n;
  } else {
   int x, y;
   std::thread t1([&]() {x = fib(n-1);});
   std::thread t2([&]() {y = fib(n-2);});
   t1.join(); t2.join();
   return (x + y);
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
