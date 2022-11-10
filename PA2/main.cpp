#include "uthread.h"
#include "Lock.h"
#include "SpinLock.h"
#include "CondVar.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;

#define UTHREAD_TIME_QUANTUM 10000
#define SHARED_BUFFER_SIZE 10
#define PRINT_FREQUENCY 100000
#define RANDOM_YIELD_PERCENT 50

static SpinLock buffer_lock;
static int item_count = 0;

void* meansOfProduction(void *arg) {
  while (true) {
    buffer_lock.lock();

    for(int i = 0; i < 100; i++)
    {
      item_count++;
    }

    cout << item_count << " items counted!" << endl;

    buffer_lock.unlock();

    // Randomly give another thread a chance
    /*
    if ((rand() % 100) < RANDOM_YIELD_PERCENT) {
      uthread_yield();
    }
    */

  }

  return nullptr;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "Usage: ./uthread-sync-demo <num_prod>" << endl;
    cerr << "Example: ./uthread-sync-demo 20" << endl;
    exit(1);
  }

  int prod_count = atoi(argv[1]);

  // Init user thread library
  int ret = uthread_init(UTHREAD_TIME_QUANTUM);
  if (ret != 0) {
    cerr << "Error: uthread_init" << endl;
    exit(1);
  }

  // Create producer threads
  int *producer_threads = new int[prod_count];
  for (int i = 0; i < prod_count; i++) {
    producer_threads[i] = uthread_create(meansOfProduction, nullptr);
    if (producer_threads[i] < 0) {
      cerr << "Error: uthread_create producer" << endl;
    }
  }

  // NOTE: Producers and consumers run until killed but if we wanted to
  //       join on them do the following

  // Wait for all producers to complete
  for (int i = 0; i < prod_count; i++) {
    int result = uthread_join(producer_threads[i], nullptr);
    if (result < 0) {
      cerr << "Error: uthread_join producer" << endl;
    }
  }

  delete[] producer_threads;

  return 0;
}
