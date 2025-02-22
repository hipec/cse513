#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include "timer.h"

#include <boost/fiber/all.hpp>

int fib1(int n) {
  if(n<2) return n;
  boost::fibers::future< int > f1(boost::fibers::async([=]() {return fib1(n-1);}));
  boost::fibers::future< int > f2(boost::fibers::async([=]() {return fib1(n-2);}));
  return f1.get() + f2.get();
}

int fib2(int n) {
  if(n<2) return n;
  std::future< int > f1(std::async([=]() {return fib2(n-1);}));
  std::future< int > f2(std::async([=]() {return fib2(n-2);}));
  return f1.get() + f2.get();
}

int main() {
  int n=20;
  timer::kernel([=]() {
    std::cout<<"Fib-fiber(" << n<< ") = "<<fib1(n)<<std::endl;
  });
  timer::kernel([=]() {
    std::cout<<"Fib-async(" << n<< ") = "<<fib2(n)<<std::endl;
  });
  return 0;
}
