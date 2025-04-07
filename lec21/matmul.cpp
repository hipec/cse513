/*
 * Author: Vivek Kumar
 * vivekk@iiitd.ac.in
 * This is a hybrid CPU+GPU implementation of matrix multiplication for Intel Integrated SoC
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

#include "timer.h"
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
  int gpu_end_row = argc>2?(atof(argv[2])*size):(0.75 * size);
  const int numcpu = argc>3?atoi(argv[3]):4;
  const int vector_width = argc>4?atoi(argv[4]):8;
  if(gpu_end_row%vector_width != 0) {
    gpu_end_row = vector_width * (gpu_end_row/vector_width);
  }
  std::cout<<"\n[USAGE]: ./matmul <square matrix size> <Fraction of computation for GPU> <Total CPU threads> <GPU vector width>\n";

  // lookup default compute device
  auto gpu = compute::system::default_device();
  // create opencl context for the device
  auto context = compute::context(gpu); 
  // create command queue for the device
  compute::command_queue queue(context, gpu);
  // print device name 
  std::cout << "device = " << gpu.name() << std::endl;
  double time_hybrid=0, time_cpu=0;
  std::cout<<"Matrix Size = "<< size << " GPU Size = " << gpu_end_row << " CPU Size = " << (size-gpu_end_row) << " CPU Threads = "<<numcpu<< " GPU vector width = " << vector_width<<std::endl;

  a = my_svm::alloc<float>(context, size*size);
  b = my_svm::alloc<float>(context, size*size);
  c = my_svm::alloc<float>(context, size*size);
  //////////////////////////////////////////////////////////////////
  //////// Calculate the sequential CPU-only execution time ////////
  //////////////////////////////////////////////////////////////////
  initialize();
  time_cpu = timer::kernel("Sequential execution", [=]() {
    for(int row=0; row<size; row++) {
      for(int col=0; col<size; col++) {
        float acc = 0.0f;
        for (int k=0; k<size; k++) {
          acc += a[row * size + k] * b[k * size + col];
        }
        c[row * size + col] = acc;
      }
    }
  });
  //////////////////////////////////////////////////////////////////
  ////////////// Set up parallel execution environment /////////////
  //////////////////////////////////////////////////////////////////
  initialize();
  // source code for the add kernel
  const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
      __kernel void matmul(__global const float *a,
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
  compute::program program = compute::program::build_with_source(source, context, "-cl-std=CL2.0");
  // create the kernel
  compute::kernel kernel(program, "matmul");
  // set the kernel arguments
  kernel.set_arg_svm_ptr(0, a);
  kernel.set_arg_svm_ptr(1, b);
  kernel.set_arg_svm_ptr(2, c);
  kernel.set_arg(3, size);
 
  timer::start_logger();
  auto start = std::chrono::system_clock::now();
  //First compute over CPU
  // Launch threads to complete the parallel execution at CPU
  std::thread *threads = new std::thread[numcpu];
  const int chunksize = (size-gpu_end_row)>0 ? ((size-gpu_end_row) / numcpu) : 0;
  for(int i=0; i<numcpu; i++) {
    int start = gpu_end_row + (chunksize * i);
    int end = (i+1)==size ? size : start + chunksize;
    threads[i] = std::thread([=]() {
      for(int row=start; row<end; row++) {
        for(int col=0; col<size; col++) {
          float acc = 0.0f;
          for (int k=0; k<size; k++) {
            acc += a[row * size + k] * b[k * size + col];
          }
          c[row * size + col] = acc;
        }
      }
    });
  }
  
  if(gpu_end_row>0) {
    // run the add kernel
    /* Total number of global work-items that would execute the kernel function */
    size_t global_size[2] = {gpu_end_row /*row size*/, size /*column size*/};
    /* Total number of local work-size that would be used to determine how to 
     * break the global work-items specified by global_size into appropriate
     *  work-group instances. If local_size is specified, the values specified 
     *  in global_size[0],... global_size[work_dim - 1] must be evenly divisable 
     *  by the corresponding values specified in local_size[0],... local_size[work_dim - 1].
     */
    size_t local_size[2] = {vector_width, vector_width};
    queue.enqueue_nd_range_kernel(kernel, 2, 0, global_size, local_size);
    queue.finish();
  }
  //block for CPU threads
  for(int i=0; i<numcpu; i++) threads[i].join();
  timer::end_logger();
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  time_hybrid = elapsed.count();
  //verify the computation
  for(int i=0; i<size*size; i++) {
    assert(c[i] == size);
  }
  std::cout<<"Test Passed \n";
  std::cout<<"Parallel Execution Speedup = "<<time_cpu/time_hybrid<<std::endl;
  //cleanup
  delete [] threads;
  my_svm::free(context, a);
  my_svm::free(context, b);
  my_svm::free(context, c);
  return 0;
}
