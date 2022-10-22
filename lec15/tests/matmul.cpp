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

#define VECTOR_WIDTH 8
#define VEC_Xf	Vec8f

float *A, *B, *C;
int size;
int num_iterations = 4;

//Instructs the compiler to turn-off vectorization for this method
__attribute__((optimize("no-tree-vectorize")))
void mul_scalar() {
  //initialize
  for(int i=0; i<size; i++) {
    for(int j=0; j<size; j++) {
      A[i*size+j] = 1.0f;
      B[i*size+j] = 1.0f;
      C[i*size+j] = 0;
    }
  }

  for(int i=0; i<size; i++) {
    for(int k=0; k<size; k++) {
      for(int j=0; j<size; j++) {
        C[i*size+j] = A[i*size+k] * B[k*size+j] + C[i*size+j];
      }
    }
  }
}

void mul_vector() {
  //initialize
  for(int i=0; i<size; i++) {
    VEC_Xf _A(1.0f);
    VEC_Xf _B(1.0f);
    VEC_Xf _C(0);
    for(int j=0; j<size; j+=VECTOR_WIDTH) {
      _A.store(&A[i*size+j]);
      _B.store(&B[i*size+j]);
      _C.store(&C[i*size+j]);
    }
  }

  for(int i=0; i<size; i++) {
    for(int k=0; k<size; k++) {
      VEC_Xf _A = A[i*size + k];
      for(int j=0; j<size; j+=VECTOR_WIDTH) {
        // C[i*size+j] = A[i*size+k] * B[k*size+j] + C[i*size+j];
        VEC_Xf _B = VEC_Xf().load(&B[k*size + j]); 
        VEC_Xf _C = VEC_Xf().load(&C[i*size + j]); 
        _C = _A*_B + _C;
        _C.store(&C[i*size + j]); 
      }
    }
  }
}

//Instructs the compiler to turn-off vectorization for this method
__attribute__((optimize("no-tree-vectorize")))
void verify() {
  int error=0;
  for(int i=0; i<size; i++) {
    for(int j=0; j<size; j++) {
      if(C[i*size+j] != size) {
        std::cout<<"ERROR: C["<<i<<", "<<j << "] = "<<C[i*size+j]<<std::endl;
        error=1;
        break;
      } 
      if(error) break;
    }
    if(error) break;
  }
}

int main(int argc, char **argv) {
  int i;
  size = argc>1?atoi(argv[1]):1024;
  assert(size % 2 == 0);
  A = new float[size*size];
  B = new float[size*size];
  C = new float[size*size];

  std::cout <<"Starting Scalar Multiplication\n";
  timer::kernel([=]() {  
    for(int i=0; i<num_iterations; i++) {
      mul_scalar();
    }
  });

  verify();

  double scalar = timer::duration();

  std::cout <<"Starting Vector Multiplication\n";
  timer::kernel([=]() {  
    for(int i=0; i<num_iterations; i++) {
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

