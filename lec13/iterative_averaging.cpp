#include<iostream>
#include<algorithm>
#include<stdio.h>
#include<string.h>
#include<cmath>
#include<sys/time.h>

/*
 * Ported from HJlib
 *
 * Author: Vivek Kumar
 *
 */

//48 * 256 * 2048
#define SIZE 25165824
#define ITERATIONS 64

double* myNew, *myVal;
int n;

long get_usecs () {
  struct timeval t;
  gettimeofday(&t,NULL);
  return t.tv_sec*1000000+t.tv_usec;
}
 
int ceilDiv(int d) {
  int m = SIZE / d;
  if (m * d == SIZE) {
    return m;
  } else {
    return (m + 1);
  }
}

void recurse(uint64_t low, uint64_t high) {
  if((high - low) > 512) {
    uint64_t mid = (high+low)/2;
    /* An async task */ recurse(low, mid);  
    recurse(mid, high);
  } else {
    for(uint64_t j=low; j<high; j++) {
      myNew[j] = (myVal[j - 1] + myVal[j + 1]) / 2.0;
    }
  }
}

void runParallel() {
  for(int i=0; i<ITERATIONS; i++) {
    recurse(1, SIZE+1);
    double* temp = myNew;
    myNew = myVal;
    myVal = temp;
  }
}

int main(int argc, char** argv) {
  myNew = new double[(SIZE + 2)];
  myVal = new double[(SIZE + 2)];
  memset(myNew, 0, sizeof(double) * (SIZE + 2));
  memset(myVal, 0, sizeof(double) * (SIZE + 2));
  myVal[SIZE + 1] = 1.0;
  long start = get_usecs();
  runParallel();
  long end = get_usecs();
  double dur = ((double)(end-start))/1000000;
  printf("Time = %.3f\n",dur);
  delete(myNew);
  delete(myVal);
}

