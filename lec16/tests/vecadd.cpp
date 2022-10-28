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
  compute::vector<int> vector_a(size, context);
  // create 'b' vector on the GPU
  compute::vector<int> vector_b(size, context);
  // create output 'c' vector on the GPU
  compute::vector<int> vector_c(size);  // TODO: Why context not passed here?
  // copy data from the host to the device
  compute::future<void> future_a = compute::copy_async(
    a, a+size, vector_a.begin(), queue
  );
  // copy data from the host to the device
  compute::future<void> future_b = compute::copy_async(
    b, b+size, vector_b.begin(), queue
  );
  // wait for copy to finish
  timer::kernel("asynchronous copy", [=]() {
    future_a.wait();
    future_b.wait();
  });
  time_gpu+=timer::duration();
  // Create function defining the body of the for-loop for carrying out vector addition
  BOOST_COMPUTE_FUNCTION(int, vector_sum, (int x, int y), {
    return x + y;
  });
  // Launch the computation on the GPU using the command queue created above
  timer::kernel("GPU kernel execution", [&]() {
    compute::transform(
      vector_a.begin(),
      vector_a.end(),
      vector_b.begin(),
      vector_c.begin(),
      vector_sum
    );
  });  
  time_gpu+=timer::duration();
  // transfer results back to the host array 'c'
  timer::kernel("copy from device", [&]() {
    compute::copy(vector_c.begin(), vector_c.end(), c);
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
  delete [] a;
  delete [] b;
  delete [] c;
  return 0;
}
