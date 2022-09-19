/**
 * This code is a modification of the original program downloaded from:
 * https://preshing.com/20120515/memory-reordering-caught-in-the-act/
 * http://preshing.com/files/ordering.zip
 */

#include <pthread.h>
#include <semaphore.h>
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
sem_t beginSema1;
sem_t beginSema2;
sem_t endSema;

int X, Y;
int r1, r2;

void *threadFunc(void *param)
{
    for (;;)
    {
        sem_wait(&beginSema1);  // Wait for signal
        wait_random();

        // ----- THE TRANSACTION! -----
        X = 1;
#ifdef USE_CPU_FENCE
        asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#else
        asm volatile("" ::: "memory");  // Prevent compiler reordering
#endif
        r1 = Y;

        sem_post(&endSema);  // Notify transaction complete
    }
    return NULL;  // Never returns
};

void *thread1Func(void *param)
{
    for (;;)
    {
        sem_wait(&beginSema1);  // Wait for signal
	wait_random();

        // ----- THE TRANSACTION! -----
        X = 1;
#ifdef USE_CPU_FENCE
        asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#else
        asm volatile("" ::: "memory");  // Prevent compiler reordering
#endif
        r1 = Y;

        sem_post(&endSema);  // Notify transaction complete
    }
    return NULL;  // Never returns
};

void *thread2Func(void *param)
{
    for (;;)
    {
        sem_wait(&beginSema2);  // Wait for signal
        wait_random(); // Random delay

        // ----- THE TRANSACTION! -----
        Y = 1;
#ifdef USE_CPU_FENCE
        asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#else
        asm volatile("" ::: "memory");  // Prevent compiler reordering
#endif
        r2 = X;

        sem_post(&endSema);  // Notify transaction complete
    }
    return NULL;  // Never returns
};

int main()
{
    // Initialize the semaphores
    sem_init(&beginSema1, 0, 0);
    sem_init(&beginSema2, 0, 0);
    sem_init(&endSema, 0, 0);

    // Spawn the threads
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, thread1Func, NULL);
    pthread_create(&thread2, NULL, thread2Func, NULL);
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
        sem_post(&beginSema1);
        sem_post(&beginSema2);
        // Wait for both threads
        sem_wait(&endSema);
        sem_wait(&endSema);
        // Check if there was a simultaneous reorder
        if (r1 == 0 && r2 == 0)
        {
            detected++;
            printf("%d reorders detected after %d iterations\n", detected, iterations);
        }
    }
    return 0;  // Never returns
}
