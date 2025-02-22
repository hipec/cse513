#include <iostream>
#include <string>
#include <thread>
#include "timer.h"

#include <boost/fiber/all.hpp>

#define millisleep(a) boost::this_fiber::sleep_for(std::chrono::milliseconds(a))
void job1() {
  std::cout << "Fiber-" << boost::this_fiber::get_id() << ": Starting Job-1" << std::endl;
  boost::this_fiber::sleep_for(std::chrono::milliseconds(500));
  std::cout << "Fiber-" << boost::this_fiber::get_id() << ": Terminating Job-1" << std::endl;
}

void job2() {
  std::cout << "Fiber-" << boost::this_fiber::get_id() << ": Starting Job-2" << std::endl;
  boost::this_fiber::sleep_for(std::chrono::milliseconds(100));
  std::cout << "Fiber-" << boost::this_fiber::get_id() << ": Terminating Job-2" << std::endl;
}

int main() {
  std::cout << "Launching execution without fibers"<<std::endl;
  timer::kernel([=]() {
    job1();
    job2();
  });
  std::cout << "Launching execution using two fibers" <<std::endl;
  timer::kernel([=]() {
    boost::fibers::fiber f1([=]() {
      job1();
    });
    boost::fibers::fiber f2([=]() {
      job2();
    });
    f1.join();
    f2.join();
  });
}
