/*
 * Author: Vivek Kumar
 * vivekk@iiitd.ac.in
 */
#include <iostream>
#include <assert.h>
#include <boost/compute/core.hpp> 
#include <boost/compute/algorithm/copy.hpp>
#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/memory_object.hpp>
#include <boost/compute/utility/source.hpp>
#include <boost/compute/svm.hpp>

#include "timer.h"
#include "svm_wrapper.h"

namespace compute = boost::compute;

int main(int argc, char** argv) {
  const int size = argc>1?atoi(argv[1]):1024*1024;
  const int vector_width = argc>2?atoi(argv[2]):8;
  double time_gpu=0;
  // lookup default compute device
  auto gpu = compute::system::default_device();
  // create opencl context for the device
  auto context = compute::context(gpu); 
  // create command queue for the device
  compute::command_queue queue(context, gpu);
  // print device name 
  std::cout << "device = " << gpu.name() << std::endl;
  int* a = my_svm::alloc<int>(context, size);
  int* b = my_svm::alloc<int>(context, size);
  int* c = my_svm::alloc<int>(context, size);
  std::fill(a, a+size, 1);
  std::fill(b, b+size, 2);
  std::fill(c, c+size, 0);
  for(int i=0; i<size; i++) assert(a[i] == 1);
  for(int i=0; i<size; i++) assert(b[i] == 2);

  // source code for the add kernel
  const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
      __kernel void add(__global const int *a,
                        __global const int *b,
                        __global int *c)
      {
          const uint i = get_global_id(0);
          c[i] = a[i] + b[i];
      }
  );
  // create the program with the source
  compute::program program = compute::program::build_with_source(source, context, "-cl-std=CL2.0");
  // create the kernel
  compute::kernel kernel(program, "add");
  // set the kernel arguments
  kernel.set_arg_svm_ptr(0, a);
  kernel.set_arg_svm_ptr(1, b);
  kernel.set_arg_svm_ptr(2, c);
  // run the add kernel
  timer::kernel("GPU kernel", [&]() {
    queue.enqueue_1d_range_kernel(kernel, 0, size, vector_width);
    queue.finish();
  });
  time_gpu+=timer::duration();
  std::cout<<"Total GPU time = "<<1000*time_gpu<<"ms"<<std::endl;
  //verify the computation
  for(int i=0; i<size; i++) assert(c[i] == 3);
  std::cout<<"Test Passed at GPU\n";
  timer::kernel("CPU kernel", [=]() {
    for(int i=0; i<size; i++) c[i] = a[i] + b[i];
  });
  std::cout<<"Speedup of GPU over CPU= "<<timer::duration()/time_gpu<<std::endl;
  //cleanup
  my_svm::free(context, a);
  my_svm::free(context, b);
  my_svm::free(context, c);
  return 0;
}
