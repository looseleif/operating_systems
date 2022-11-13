#include "uthread.h"
#include "Lock.h"
#include "CondVar.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <chrono>

using namespace std;

#define UTHREAD_TIME_QUANTUM 10000
#define SHARED_BUFFER_SIZE 10
#define PRINT_FREQUENCY 100000
#define RANDOM_YIELD_PERCENT 50

// Shared buffer
static int item_count = 0;

// Shared buffer synchronization
static Lock buffer_lock;

// Bookkeeping
static bool producer_in_critical_section = false;

void* testFunction(void *arg) {
    buffer_lock.lock();

    // Make sure synchronization is working correctly
    assert(!producer_in_critical_section);
    producer_in_critical_section = true;
    
    for(int i = 0; i < 100; i++)
    {
    	item_count++;
    }

    producer_in_critical_section = false;
    buffer_lock.unlock();

  return nullptr;
}



int main(int argc, char *argv[]) {
  
  using namespace std::chrono;
  if (argc != 2) {
    cerr << "Usage: ./uthread-sync-demo <num_threads>" << endl;
    cerr << "Example: ./uthread-sync-demo 20" << endl;
    exit(1);
  }

  int lock_count = atoi(argv[1]);

  if ((lock_count) > 99) {
    cerr << "Error: <num_producer> + <num_consumer> must be <= 99" << endl;
    exit(1);
  }

  // Init user thread library
  int ret = uthread_init(UTHREAD_TIME_QUANTUM);
  if (ret != 0) {
    cerr << "Error: uthread_init" << endl;
    exit(1);
  }
  
  //start lock timer
  high_resolution_clock::time_point lock1 = high_resolution_clock::now();
  
  // Create producer threads
  int *lock_threads = new int[lock_count];
  for (int i = 0; i < lock_count; i++) {
    lock_threads[i] = uthread_create(testFunction, nullptr);
    if (lock_threads[i] < 0) {
      cerr << "Error: uthread_create producer" << endl;
    }
  }
  
  //join the lock threads
  for (int i = 0; i < lock_count; i++) {
    int result = uthread_join(lock_threads[i], nullptr);
    if (result < 0) {
      cerr << "Error: uthread_join producer" << endl;
    }
  }
  
  high_resolution_clock::time_point lock2 = high_resolution_clock::now();
  
  duration<double> time_span = duration_cast<duration<double>>(lock2 - lock1);
  
  cout << "Item count: " << item_count << endl;
  cout << "Time required: " << time_span.count() << endl;

  delete[] lock_threads;

  return 0;
}
