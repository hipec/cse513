/*
 * Author: Vivek Kumar
 * vivekk@iiitd.ac.in
 */
#include<iostream>
#include<stdio.h>
#include<assert.h>
#include<string.h>
#include "vectorclass.h"
#include "timer.h"

#define VECTOR_WIDTH 4
#define VEC_Xf  Vec4f

float *A, *B, *C;
int size;
int num_iterations = 10;

//Instructs the compiler to turn-off vectorization for this method
__attribute__((optimize("no-tree-vectorize")))
void reset_scalar() {
  //initialize
  for(int i=0; i<size; i++) {
    A[i] = 0;
    B[i] = 0.1f;
    C[i] = 0.1f;
  }
}

//Instructs the compiler to turn-off vectorization for this method
__attribute__((optimize("no-tree-vectorize")))
void mul_scalar() {
  for(int i=0; i<size; i++) {
    A[i] = B[i] * C[i];
  }
}

void reset_vector() {
  //initialize
  VEC_Xf _A(0), _B(0.1f), _C(0.1f);
  for(int i=0; i<size; i+=VECTOR_WIDTH) {
    _A.store(A+i);
    _B.store(B+i);
    _C.store(C+i);
  }
}

void mul_vector() {
  //initialize
  VEC_Xf _A;
  for(int i=0; i<size; i+=VECTOR_WIDTH) {
    VEC_Xf _B = VEC_Xf().load(B+i);
    VEC_Xf _C = VEC_Xf().load(C+i);
    _A = _B * _C;
    _A.store(A+i);
  }
}

void verify() {
  for(int i=0; i<size; i++) {
    if(A[i] != 0.01f) {
      std::cout<<"ERROR: A["<<i<<"] = "<<A[i]<<std::endl;
      break; 
    }
  }
}

int main(int argc, char **argv) {
  int i;
  size = argc>1?atoi(argv[1]):1024000;
  assert(size % 2 == 0);
  A = new float[size];
  B = new float[size];
  C = new float[size];

  std::cout <<"Starting Scalar Multiplication\n";
  timer::kernel([=]() {  
    for(int i=0; i<num_iterations; i++) {
      reset_scalar();
      mul_scalar();
    }
  });

  verify();

  double scalar = timer::duration();

  std::cout <<"Starting Vector Multiplication\n";
  timer::kernel([=]() {  
    for(int i=0; i<num_iterations; i++) {
      reset_vector();
      mul_vector();
    }
  });

  verify();

  double vector = timer::duration();
  std::cout<<"Speedup = "<<(scalar * 1.0f) /vector <<std::endl;

  delete [] A;
  delete [] B;
  delete [] C;

  return 0;
}

