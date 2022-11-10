/*
 * Author: Vivek Kumar
 * vivekk@iiitd.ac.in
 *
 * GPU-only matmul with different amount of parallelism inside work-items
 */
#include <iostream>
#include <thread>
#include <assert.h>
#include <boost/compute/core.hpp>
#include <boost/compute/algorithm/copy.hpp>
#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/memory_object.hpp>
#include <boost/compute/utility/source.hpp>
#include <boost/compute/svm.hpp>

#include <chrono>
#include "svm_wrapper.h"

namespace compute = boost::compute;

size_t size;
float *a, *b, *c;

void initialize() {
  for(int i=0; i<size*size; i++) {
      a[i] = 1.0f;
      b[i] = 1.0f;
      c[i] = 0;
  }
}

int main(int argc, char** argv) {
  size = argc>1?atoi(argv[1]):1024;
  const int vector_width = argc>2?atoi(argv[2]):8;
  std::cout<<"\n[USAGE]: ./gpu_matmul <square matrix size> <GPU vector width>\n";

  // lookup default compute device
  auto gpu = compute::system::default_device();
  // create opencl context for the device
  auto context = compute::context(gpu); 
  // create command queue for the device
  compute::command_queue queue(context, gpu);
  // print device name 
  std::cout << "device = " << gpu.name() << std::endl;
  double time_v1=0, time_v2=0;
  std::cout<<"Matrix Size = "<< size << " GPU vector width = " << vector_width<<std::endl;

  a = my_svm::alloc<float>(context, size*size);
  b = my_svm::alloc<float>(context, size*size);
  c = my_svm::alloc<float>(context, size*size);
  //////////////////////////////////////////////////////////////////
  ////////////// Set up parallel execution environment /////////////
  //////////////////////////////////////////////////////////////////
  // source code for the add kernel
  // Each work-item computes the k-loop only O(N)
  const char source_v1[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
      __kernel void matmul_v1(__global const float *a,
                        __global const float *b,
                        __global float *c,
			const int size) {
          const int row = get_global_id(0);
          const int col = get_global_id(1);
          float acc = 0.0f;
          for (int k=0; k<size; k++) {
            acc += a[row * size + k] * b[k * size + col];
          }
          c[row * size + col] = acc;
      }
  );
  // create the program with the source and compile
  compute::program program_v1 = compute::program::build_with_source(source_v1, context, "-cl-std=CL2.0");
  // create the kernel
  compute::kernel kernel_v1(program_v1, "matmul_v1");
  // set the kernel arguments
  kernel_v1.set_arg_svm_ptr(0, a);
  kernel_v1.set_arg_svm_ptr(1, b);
  kernel_v1.set_arg_svm_ptr(2, c);
  kernel_v1.set_arg(3, size);
 
  // Each work-item computes the j-loop and k-loop O(N^2)
  const char source_v2[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
      __kernel void matmul_v2(__global const float *a,
                        __global const float *b,
                        __global float *c,
			const int size) {
         const int row = get_global_id(0);
	 for(int col=0; col<size; col++) {
           float acc = 0.0f;
	   for (int k=0; k<size; k++) {
	     acc += a[row * size + k] * b[k * size + col];
	   } 
           c[row * size + col] = acc;
	 }
      }
  );

  // create the program with the source and compile
  compute::program program_v2 = compute::program::build_with_source(source_v2, context, "-cl-std=CL2.0");
  // create the kernel
  compute::kernel kernel_v2(program_v2, "matmul_v2");
  // set the kernel arguments
  kernel_v2.set_arg_svm_ptr(0, a);
  kernel_v2.set_arg_svm_ptr(1, b);
  kernel_v2.set_arg_svm_ptr(2, c);
  kernel_v2.set_arg(3, size);
  { // Test version V1 
    initialize();
    // run the add kernel
    /* Total number of global work-items that would execute the kernel function */
    size_t global_size[2] = {size /*row size*/, size /*column size*/};
    /* Total number of local work-size that would be used to determine how to 
     * break the global work-items specified by global_size into appropriate
     *  work-group instances. If local_size is specified, the values specified 
     *  in global_size[0],... global_size[work_dim - 1] must be evenly divisable 
     *  by the corresponding values specified in local_size[0],... local_size[work_dim - 1].
     */
    size_t local_size[2] = {vector_width, vector_width};
    auto start = std::chrono::system_clock::now(); 
    queue.enqueue_nd_range_kernel(kernel_v1, 2, 0, global_size, local_size);
    queue.finish();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    time_v1= elapsed.count();
    std::cout<<"Time for version-1: " << time_v1*1000 <<"ms\n";
    //verify the computation
    for(int i=0; i<size*size; i++) {
      assert(c[i] == size);
    }
  }

  { // Test version V1 
    initialize();
    auto start = std::chrono::system_clock::now(); 
    // run the add kernel
    queue.enqueue_1d_range_kernel(kernel_v2, 0, size, vector_width);
    queue.finish();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    time_v2 = elapsed.count();
    std::cout<<"Time for version-2: " << time_v2*1000 <<"ms\n";
    //verify the computation
    for(int i=0; i<size*size; i++) {
      assert(c[i] == size);
    }
  }
  std::cout<<"Test Passed \n";
  std::cout<<"Speedup of V2 over V1 = "<<time_v2/time_v1<<std::endl;
  //cleanup
  my_svm::free(context, a);
  my_svm::free(context, b);
  my_svm::free(context, c);
  return 0;
}
