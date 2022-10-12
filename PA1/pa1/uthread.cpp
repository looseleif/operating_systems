#include "uthread.h"
#include "TCB.h"
#include <cassert>
#include <deque>
#include <map>

using namespace std;

// Finished queue entry type
typedef struct finished_queue_entry {
  TCB *tcb;             // Pointer to TCB
  void *result;         // Pointer to thread result (output)
} finished_queue_entry_t;

// Join queue entry type
typedef struct join_queue_entry {
  TCB *tcb;             // Pointer to TCB
  int waiting_for_tid;  // TID this thread is waiting on
} join_queue_entry_t;

// You will need to maintain structures to track the state of threads
// - uthread library functions refer to threads by their TID so you will want
//   to be able to access a TCB given a thread ID
// - Threads move between different states in their lifetime (READY, BLOCK,
//   FINISH). You will want to maintain separate "queues" (doesn't have to
//   be that data structure) to move TCBs between different thread queues.
//   Starter code for a ready queue is provided to you
// - Separate join and finished "queues" can also help when supporting joining.
//   Example join and finished queue entry types are provided above

// Queues
static deque<TCB*> ready_queue;

static int global_thread_count = 0;

static struct itimerval value;

//static map<int,TCB*> thread_map;

static TCB* thread_translation[MAX_THREAD_NUM] ;


// Interrupt Management --------------------------------------------------------

// Start a countdown timer to fire an interrupt
static void startInterruptTimer()
{

        setitimer(ITIMER_VIRTUAL, &value, NULL);

}

// Block signals from firing timer interrupt
static void disableInterrupts()
{
        // TODO
}

// Unblock signals to re-enable timer interrupt
static void enableInterrupts()
{
        // TODO
}

void signalHandler(int signal_num){

        printf("ajhsdkj");
        return;

}


// Queue Management ------------------------------------------------------------

// Add TCB to the back of the ready queue
void addToReadyQueue(TCB *tcb)
{
        ready_queue.push_back(tcb);
}

// Removes and returns the first TCB on the ready queue
// NOTE: Assumes at least one thread on the ready queue
TCB* popFromReadyQueue()
{
        assert(!ready_queue.empty());

        TCB *ready_queue_head = ready_queue.front();
        ready_queue.pop_front();
        return ready_queue_head;
}

// Removes the thread specified by the TID provided from the ready queue
// Returns 0 on success, and -1 on failure (thread not in ready queue)
int removeFromReadyQueue(int tid)
{
        for (deque<TCB*>::iterator iter = ready_queue.begin(); iter != ready_queue.end(); ++iter)
        {
                if (tid == (*iter)->getId())
                {
                        ready_queue.erase(iter);
                        return 0;
                }
        }

        // Thread not found
        return -1;
}

// Helper functions ------------------------------------------------------------

// Switch to the next ready thread
static void switchThreads()
{
    
//     // currentThread is a global variable shared by both threads
//     static int currentThread = 0;

//     // flag is a local stack variable to each thread
//     volatile int flag = 0;

//     // getcontext() will "return twice" - Need to differentiate between the two
//     int ret_val = getcontext(&cont[currentThread]);
//     cout << "SWITCH: currentThread = " << currentThread << endl;

//     // If flag == 1 then it was already set below so this is the second return
//     // from getcontext (run this thread)
//     if (flag == 1) {
//         return;
//     }

//     // This is the first return from getcontext (switching threads)
//     flag = 1;
//     currentThread = 1 - currentThread;
//     setcontext(&cont[currentThread]);
}


// Library functions -----------------------------------------------------------

// The function comments provide an (incomplete) summary of what each library
// function must do

// Starting point for thread. Calls top-level thread function
void stub(void *(*start_routine)(void *), void *arg)
{
        
        uthread_exit(start_routine(arg));

}

int uthread_init(int quantum_usecs)
{
        // Initialize any data structures
        // Setup timer interrupt and handler
        // Create a thread for the caller (main) thread

        value.it_value.tv_sec = 0;
        value.it_value.tv_usec = quantum_usecs;
        value.it_interval.tv_sec = 0;
        value.it_interval.tv_usec = quantum_usecs;

        signal(SIGVTALRM, signalHandler);

        startInterruptTimer();

        static ucontext_t cont;

        return getcontext(&cont);

}

int uthread_create(void* (*start_routine)(void*), void* arg)
{
        // Create a new thread and add it to the ready queue

        TCB* thread_instance = new TCB(global_thread_count, start_routine, arg, READY);

        cout << thread_instance->getId() << endl;
        //assert(global_thread_count==0);

        addToReadyQueue(thread_instance);

        return global_thread_count++;
}

int uthread_join(int tid, void **retval)
{
        // If the thread specified by tid is already terminated, just return
        // If the thread specified by tid is still running, block until it terminates
        // Set *retval to be the result of thread if retval != nullptr
        return 0;
}

int uthread_yield(void)
{
        // TODO
        return 0;
}

void uthread_exit(void *retval)
{
        // If this is the main thread, exit the program
        // Move any threads joined on this thread back to the ready queue
        // Move this thread to the finished queue
        return;
}

int uthread_suspend(int tid)
{
        // Move the thread specified by tid from whatever state it is
        // in to the block queue
        return 0;
}

int uthread_resume(int tid)
{
        // Move the thread specified by tid back to the ready queue
        return 0;
}

int uthread_self()
{
        // TODO
        return 0;
}

int uthread_get_total_quantums()
{
        // TODO
        return 0;
}

int uthread_get_quantums(int tid)
{
        // TODO
        return 0;
}
