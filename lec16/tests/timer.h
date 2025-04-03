#include <chrono>
#define millisleep(a) boost::this_fiber::sleep_for(std::chrono::milliseconds(a))
namespace timer {
  static double lasttime=0;
  template<typename T>
  void kernel(const std::string& str, T &&lambda) {
    auto start = std::chrono::system_clock::now();
    lambda();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    lasttime = elapsed.count();
    std::cout<<"Time("<<str<<") = "<<elapsed.count() * 1000 << " ms" << std::endl;
  }
  double duration() { return lasttime; }
}
