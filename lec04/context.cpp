#include <cstdlib>
#include <iostream>

#include <boost/context/continuation.hpp>

namespace ctx = boost::context;

ctx::continuation A(ctx::continuation && c) {
    std::cout << "A: Entered" << std::endl;
    c = c.resume();
    std::cout << "A: Exiting" << std::endl;
    return std::move( c);
}

ctx::continuation B(ctx::continuation && c) {
    std::cout << "B: Entered" << std::endl;
    c = c.resume();
    std::cout << "B: Exiting" << std::endl;
    return std::move( c);
}

ctx::continuation C(ctx::continuation && c) {
    std::cout << "C: Entered" << std::endl;
    c = c.resume();
    std::cout << "C: Exiting" << std::endl;
    return std::move( c);
}

int main() {
    ctx::continuation a = ctx::callcc(A);
    ctx::continuation b = ctx::callcc(B);
    ctx::continuation c = ctx::callcc(C);
    a.resume();
    b.resume();
    c.resume();
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
