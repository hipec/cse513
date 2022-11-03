#include <chrono>
#define millisleep(a) boost::this_fiber::sleep_for(std::chrono::milliseconds(a))
namespace timer {
  template<typename T>
  double kernel(const std::string& str, T &&lambda) {
    auto start = std::chrono::system_clock::now();
    lambda();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout<<"Time("<<str<<") = "<<elapsed.count() * 1000 << " ms" << std::endl;
    return elapsed.count();
  }
}
