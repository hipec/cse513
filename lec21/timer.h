#include <chrono>
#include "cpucounters.h" //PCM related: https://github.com/intel/pcm
#include "utils.h"       //PCM related: https://github.com/intel/pcm

#define millisleep(a) boost::this_fiber::sleep_for(std::chrono::milliseconds(a))

namespace timer {
  static double lasttime=0;
  template<typename T>
  double kernel(const std::string& str, T &&lambda) {
    auto start = std::chrono::system_clock::now();
    lambda();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    lasttime = elapsed.count();
    std::cout<<"Time("<<str<<") = "<<elapsed.count() * 1000 << " ms" << std::endl;
    return lasttime; 
  }
  double duration() { return lasttime; }

  static PCM *___pcm;
  SystemCounterState ___before_sstate, ___after_sstate;
  double start_time;
  static int init=0;

  void start_logger() {
    if(init) return;
    ___pcm = PCM::getInstance();
    if (init==0) {
        ___pcm->resetPMU();
	init=1;
    }
    // program() creates common semaphore for the singleton, so ideally to be called before any other references to PCM
    PCM::ErrorCode status = ___pcm->program();

    switch (status)
    {
    case PCM::Success:
	//std::cerr << "Success initializing PCM" << std::endl;
        break;
    case PCM::MSRAccessDenied:
        std::cerr << "Access to Processor Counter Monitor has denied (no MSR or PCI CFG space access)." << std::endl;
        exit(EXIT_FAILURE);
    case PCM::PMUBusy:
        std::cerr << "Access to Processor Counter Monitor has denied (Performance Monitoring Unit is occupied by other application). Try to stop the application that uses PMU." << std::endl;
        std::cerr << "Alternatively you can try running PCM with option -r to reset PMU configuration at your own risk." << std::endl;
        exit(EXIT_FAILURE);
    default:
        std::cerr << "Access to Processor Counter Monitor has denied (Unknown error)." << std::endl;
        exit(EXIT_FAILURE);
    }
    //print_cpu_details();
    auto time_now = std::chrono::system_clock::now();
    auto seconds = std::chrono::duration<double>(time_now.time_since_epoch());
    start_time = seconds.count();
    ___before_sstate = getSystemCounterState();
  }

  void end_logger() {
    ___after_sstate = getSystemCounterState();
    auto time_now = std::chrono::system_clock::now();
    auto seconds = std::chrono::duration<double>(time_now.time_since_epoch());
    double end = seconds.count();
    double _duration = end - start_time;
    double _joules = getConsumedJoules(___before_sstate, ___after_sstate);
    ___pcm->cleanup();
    std::cout<<"STATISTICS: Time = " <<_duration*1000<<" ms\n";
    std::cout<<"STATISTICS: Energy = " <<_joules<<" joules\n";
    std::cout<<"STATISTICS: EDP = " <<_joules*_duration<<" (Lower the Better)\n";
  }
} //namespace timer

