#include <cstdlib>
#include <iostream>

#include <boost/context/continuation.hpp>

namespace ctx = boost::context;

int main() {
  int counter = 1;
  ctx::continuation c = ctx::callcc([&](ctx::continuation && c) {
    counter += 1;
    c = c.resume();
    counter -= 1;
    return std::move(c);
  });
  
  counter += 1;
  c = c.resume();
  std::cout << "Counter = " << counter<< std::endl;
  return 0;
}                


