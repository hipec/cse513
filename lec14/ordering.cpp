/**
 * This code is a modification of the original program downloaded from:
 * https://preshing.com/20120515/memory-reordering-caught-in-the-act/
 * http://preshing.com/files/ordering.zip
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <random>

#ifdef USE_SINGLE_HW_THREAD
#include <sched.h>
#endif


std::random_device rd; // obtain a random number from hardware
std::mt19937 gen(rd()); // seed the generator
std::uniform_int_distribution<> distr(1000, 100000);
inline void wait_random() {
  for(int i=0; i<distr(gen); i++);
}

//-------------------------------------
//  Main program, as decribed in the post
//-------------------------------------
pthread_mutex_t m1=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m=PTHREAD_MUTEX_INITIALIZER;

volatile int counter1 = 0;
volatile int counter2 = 0;
volatile int counter = 0;
volatile int shutdown = 0;

pthread_cond_t c1=PTHREAD_COND_INITIALIZER;
pthread_cond_t c2=PTHREAD_COND_INITIALIZER;
pthread_cond_t c=PTHREAD_COND_INITIALIZER;

void wait(int id) {
  switch(id) {
    case 0:
      pthread_mutex_lock(&m);
      while(counter != 2) {
        pthread_cond_wait(&c, &m);
      }
      counter=0;
      pthread_mutex_unlock(&m);
      break;
    case 1:
      pthread_mutex_lock(&m1);
      while(counter1 == 0) {
        pthread_cond_wait(&c1, &m1);
      }
      counter1=0;
      pthread_mutex_unlock(&m1);
      break;
    case 2:
      pthread_mutex_lock(&m2);
      while(counter2 == 0) {
        pthread_cond_wait(&c2, &m2);
      }
      counter2=0;
      pthread_mutex_unlock(&m2);
      break;
  }
  wait_random();
}

void signal(int id) {
  switch(id) {
    case 0:
      pthread_mutex_lock(&m);
      counter++;
      pthread_cond_signal(&c);
      pthread_mutex_unlock(&m);
      break;
    case 1:
      pthread_mutex_lock(&m1);
      counter1=1;
      pthread_cond_signal(&c1);
      pthread_mutex_unlock(&m1);
      break;
    case 2:
      pthread_mutex_lock(&m2);
      counter2=1;
      pthread_cond_signal(&c2);
      pthread_mutex_unlock(&m2);
      break;
  }
}

int X, Y;
int r1, r2;

void *threadFunc(void *param) {
  int id = *((int*)param);
  while(!shutdown) {
    wait(id);
    switch(id) {
      case 1:
	X = 1;
#ifdef USE_CPU_FENCE
        asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#else
        asm volatile("" ::: "memory");  // Prevent compiler reordering
#endif
        r1 = Y;
        break;
      case 2:
	Y = 1;
#ifdef USE_CPU_FENCE
        asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#else
        asm volatile("" ::: "memory");  // Prevent compiler reordering
#endif
        r2 = X;
        break;
    }
    signal(0);
  }
  return NULL;  // Never returns
}

int main()
{
    // Spawn the threads
    pthread_t thread1, thread2;
    int id1=1, id2=2;
    pthread_create(&thread1, NULL, threadFunc, ((void*)&id1));
    pthread_create(&thread2, NULL, threadFunc, ((void*)&id2));
    usleep(500);
#ifdef USE_SINGLE_HW_THREAD
    // Force thread affinities to the same cpu core.
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(0, &cpus);
    pthread_setaffinity_np(thread1, sizeof(cpu_set_t), &cpus);
    pthread_setaffinity_np(thread2, sizeof(cpu_set_t), &cpus);
#endif

    // Repeat the experiment ad infinitum
    int detected = 0;
    for (int iterations = 1; iterations<2000 ; iterations++)
    {
        // Reset X and Y
        X = 0;
        Y = 0;
        // Signal both threads
        signal(1);
	signal(2);
        // Wait for both threads
	wait(0);
        // Check if there was a simultaneous reorder
        if (r1 == 0 && r2 == 0)
        {
            detected++;
            printf("%d reorders detected after %d iterations\n", detected, iterations);
        }
    }
    shutdown=1;
    // Signal both threads
    signal(1);
    signal(2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;  // Never returns
}
