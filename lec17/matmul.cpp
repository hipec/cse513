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

#include <thread>

#include "timer.h"

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
  std::cout<<"[USAGE]: ./matmul <square matrix size> <Fraction of computation for GPU> <Total CPU threads> <GPU vector width>\n";
  double time_hybrid=0, time_cpu=0;
  std::cout<<"Matrix Size = "<< size << " GPU Size = " << gpu_end_row << " CPU Size = " << (size-gpu_end_row) << " CPU Threads = "<<numcpu<< " GPU vector width = " << vector_width<<std::endl;
  a = new float[size*size];
  b = new float[size*size];
  c = new float[size*size];
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
  // lookup default compute device
  auto gpu = compute::system::default_device();
  // create opencl context for the device
  auto context = compute::context(gpu); 
  // create command queue for the device
  compute::command_queue queue(context, gpu);
  // print device name 
  std::cout << "device = " << gpu.name() << std::endl;
  // create 'a' vector on the GPU
  compute::buffer vector_a(context, size * size * sizeof(float), compute::memory_object::mem_flags::use_host_ptr, a);
  // create 'b' vector on the GPU
  compute::buffer vector_b(context, size * size * sizeof(float), compute::memory_object::mem_flags::use_host_ptr, b);
  // create output 'c' vector on the GPU
  compute::buffer vector_c(context, size * size * sizeof(float), compute::memory_object::mem_flags::use_host_ptr, c);
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
  compute::program program = compute::program::build_with_source(source, context);
  // create the kernel
  compute::kernel kernel(program, "matmul");
  // set the kernel arguments
  kernel.set_args(vector_a, vector_b, vector_c, size);
 
  auto start = std::chrono::system_clock::now(); 
  //First compute over CPU
  // Launch threads to complete the parallel execution at CPU
  std::thread *threads = new std::thread[numcpu];
  const int chunksize = (size-gpu_end_row) / numcpu;
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
    // write the data from 'a' and 'b' to the device
    queue.enqueue_write_buffer(vector_a, 0, size * size * sizeof(float), a);
    queue.enqueue_write_buffer(vector_b, 0, size * size * sizeof(float), b);
    // run the add kernel
    size_t global_size[2] = {gpu_end_row, size};
    size_t local_size[2] = {vector_width, vector_width};
    queue.enqueue_nd_range_kernel(kernel, 2, 0, global_size, local_size);
    queue.enqueue_read_buffer(vector_c, 0, size * gpu_end_row * sizeof(float), c); //blocking communication
  }
  //block for CPU threads
  for(int i=0; i<numcpu; i++) threads[i].join();
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout<<"Time = "<<elapsed.count() * 1000 << " ms" << std::endl;
  time_hybrid = elapsed.count();
  //verify the computation
  for(int i=0; i<size*size; i++) {
    assert(c[i] == size);
  }
  std::cout<<"Test Passed \n";
  std::cout<<"Parallel Execution Speedup = "<<time_cpu/time_hybrid<<std::endl;
  //cleanup
  delete [] threads;
  delete [] a;
  delete [] b;
  delete [] c;
  return 0;
}
