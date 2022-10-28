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

#include "timer.h"

namespace compute = boost::compute;

int main(int argc, char** argv) {
  int size = argc>1?atoi(argv[1]):1024*1024;
  double time_gpu=0;
  // lookup default compute device
  auto gpu = compute::system::default_device();
  // create opencl context for the device
  auto context = compute::context(gpu); 
  // create command queue for the device
  compute::command_queue queue(context, gpu);
  // print device name 
  std::cout << "device = " << gpu.name() << std::endl;
  int* a = new int[size];
  int* b = new int[size];
  int* c = new int[size];
  std::fill(a, a+size, 1);
  std::fill(b, b+size, 2);
  // create 'a' vector on the GPU
  compute::buffer vector_a(context, size * sizeof(int), compute::memory_object::mem_flags::use_host_ptr, (void*) a);
  // create 'b' vector on the GPU
  compute::buffer vector_b(context, size * sizeof(int), compute::memory_object::mem_flags::use_host_ptr, (void*) b);
  // create output 'c' vector on the GPU
  compute::buffer vector_c(context, size * sizeof(int), compute::memory_object::mem_flags::use_host_ptr, (void*) c);
  // source code for the add kernel
  const char source[] =
      "__kernel void add(__global const float *a,"
      "                  __global const float *b,"
      "                  __global float *c)"
      "{"
      "    const uint i = get_global_id(0);"
      "    c[i] = a[i] + b[i];"
      "}";
  // create the program with the source
  compute::program program = compute::program::create_with_source(source, context);
  // compile the program
  program.build();
  // create the kernel
  compute::kernel kernel(program, "add");
  // set the kernel arguments
  kernel.set_arg(0, vector_a);
  kernel.set_arg(1, vector_b);
  kernel.set_arg(2, vector_c);
  // write the data from 'a' and 'b' to the device
  timer::kernel("data transfer to device", [&]() {
    queue.enqueue_write_buffer(vector_a, 0, size * sizeof(int), a);
    queue.enqueue_write_buffer(vector_b, 0, size * sizeof(int), b);
  });
  time_gpu+=timer::duration();
  // run the add kernel
  timer::kernel("GPU kernel", [&]() {
    queue.enqueue_1d_range_kernel(kernel, 0, size, 0);
  });
  time_gpu+=timer::duration();
  timer::kernel("copy c buffer", [&]() {
    queue.enqueue_read_buffer(vector_c, 0, size * sizeof(int), c);
  });
  time_gpu+=timer::duration();
  std::cout<<"Total GPU time = "<<1000*time_gpu<<"ms"<<std::endl;
  //verify the computation
  for(int i=0; i<size; i++) assert(c[i] == 3);
  std::cout<<"Test Passed at GPU\n";
  timer::kernel("CPU kernel", [=]() {
    for(int i=0; i<size; i++) c[i] = a[i] + b[i];
  });
  //cleanup
  delete [] a;
  delete [] b;
  delete [] c;
  return 0;
}
