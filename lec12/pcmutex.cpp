#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <boost/fiber/all.hpp>

std::string str;
boost::fibers::mutex mtx;
boost::fibers::condition_variable cnd_count;

int main() {
  boost::fibers::fiber f1([&]() {
    std::unique_lock<boost::fibers::mutex> lck (mtx);
    if(str.size() ==0) {
      std::cout<<"F1 wait in\n";
      cnd_count.wait(lck);  
      std::cout<<"F1 wait out\n";
    }
    std::cout<<str<<std::endl;
  });

  boost::fibers::fiber f2([&]() {
    std::cout<<"F2 in\n";
    std::unique_lock<boost::fibers::mutex> lck (mtx);
    str = "Hello Fiber"; 
    std::cout<<"F2 notify in\n";
    cnd_count.notify_one();  
    std::cout<<"F2 notify done\n";
  });
  f1.join(); 
  f2.join();
  return 0;
}
