#include<iostream>
#include<stdio.h>
#include<assert.h>
#include<string.h>
#include "vectorclass.h"
#include "timer.h"

#define VECTOR_WIDTH 4
#define VEC_Xi	Vec4i

int *A, *B, *C;
int size;
int num_iterations = 10;

//Instructs the compiler to turn-off vectorization for this method
__attribute__((optimize("no-tree-vectorize")))
void reset_scalar() {
  //initialize
  for(int i=0; i<size; i++) {
    A[i] = 0;
    B[i] = 1;
    C[i] = 1;
  }
}

//Instructs the compiler to turn-off vectorization for this method
__attribute__((optimize("no-tree-vectorize")))
void add_scalar() {
  for(int i=0; i<size; i++) {
    A[i] = B[i] + C[i];
  }
}

void reset_vector() {
  //initialize
  VEC_Xi _A(0), _B(1), _C(1);
  for(int i=0; i<size; i+=VECTOR_WIDTH) {
    _A.store(A+i);
    _B.store(B+i);
    _C.store(C+i);
  }
}

void add_vector() {
  //initialize
  VEC_Xi _A;
  for(int i=0; i<size; i+=VECTOR_WIDTH) {
    VEC_Xi _B = VEC_Xi().load(B+i);
    VEC_Xi _C = VEC_Xi().load(C+i);
    _A = _B + _C;
    _A.store(A+i);
  }
}

void verify() {
  for(int i=0; i<size; i++) {
    if(A[i] != 2) {
      std::cout<<"ERROR: A["<<i<<"] = "<<A[i]<<std::endl;
      break; 
    }
  }
}

int main(int argc, char **argv) {
  int i;
  size = argc>1?atoi(argv[1]):1024000;
  assert(size % 2 == 0);
  A = new int[size];
  B = new int[size];
  C = new int[size];

  std::cout <<"Starting Scalar Addition\n";
  timer::kernel([=]() {  
    for(int i=0; i<num_iterations; i++) {
      reset_scalar();
      add_scalar();
    }
  });

  verify();

  double scalar = timer::duration();

  std::cout <<"Starting Vector Addition\n";
  timer::kernel([=]() {  
    for(int i=0; i<num_iterations; i++) {
      reset_vector();
      add_vector();
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

