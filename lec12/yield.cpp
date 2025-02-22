#include <iostream>
#include <string>
#include <thread>

#include <boost/fiber/all.hpp>

int main() {
  boost::fibers::fiber f1([=]() {
    std::cout<<"A ";
    boost::this_fiber::yield();
    std::cout<<"B ";
    boost::this_fiber::yield();
    std::cout<<"C ";
  });
  boost::fibers::fiber f2([=]() {
    std::cout<<"D ";
    boost::this_fiber::yield();
    std::cout<<"E ";
    boost::this_fiber::yield();
    std::cout<<"F ";
  });
  f1.join(); 
  f2.join();
  return 0;
}

