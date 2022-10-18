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
static deque<TCB*> block_queue; //may not be necessary?
static deque<finished_queue_entry_t*> finish_queue;
static deque<join_queue_entry_t*> join_queue;

static int global_thread_count = 0;

static TCB* current_thread;

static struct itimerval tv;

static TCB* thread_translation[MAX_THREAD_NUM];


// Interrupt Management --------------------------------------------------------

// Start a countdown timer to fire an interrupt
static void startInterruptTimer()
{

        setitimer(ITIMER_VIRTUAL, &tv, NULL);

}

// Block signals from firing timer interrupt
static struct sigaction _sigAction;

static bool interrupts_enabled = true;

static void disableInterrupts()
{
        cout << "disable int" << endl;
        //assert(interrupts_enabled);
	//sigprocmask(SIG_BLOCK,&_sigAction.sa_mask, NULL);
        interrupts_enabled = false;
}

// Unblock signals to re-enable timer interrupt
static void enableInterrupts()
{
        cout << "enabling int" << endl;
        assert(!interrupts_enabled);
        interrupts_enabled = true;
	sigprocmask(SIG_UNBLOCK,&_sigAction.sa_mask, NULL);
}

void time(int signal_num){

        cout << "interrupt has run" << endl;
        uthread_yield();
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
    // flag is a local stack variable to each thread
    volatile int flag = 0;

    // getcontext() will "return twice" - Need to differentiate between the two
    int ret_val = current_thread->saveContext();
    //cout << "SWITCH: currentThread = " << currentThread << endl;
    cout << flag << " flag" << endl;
    // If flag == 1 then it was already set below so this is the second return
    // from getcontext (run this thread)
    if (flag == 1) {
        enableInterrupts();
        return;
    }

    // This is the first return from getcontext (switching threads)
    flag = 1;
    if(current_thread->getState() != BLOCK)
    {
        addToReadyQueue(current_thread);
        current_thread->setState(READY);
    }
    current_thread = popFromReadyQueue();
    current_thread->setState(RUNNING);
    current_thread->loadContext();

}


// Library functions -----------------------------------------------------------

// The function comments provide an (incomplete) summary of what each library
// function must do

// Starting point for thread. Calls top-level thread function
void stub(void *(*start_routine)(void *), void *arg)
{
        // enable intrrpt
        cout << "thread " << current_thread->getId() << " executing worker" << endl;
        enableInterrupts();
        uthread_exit(start_routine(arg));

}

int uthread_init(int quantum_usecs)
{

        signal(SIGVTALRM, time);

        tv.it_value.tv_sec = 0;
        tv.it_value.tv_usec = quantum_usecs;
        tv.it_interval.tv_sec = 0;
        tv.it_interval.tv_usec = quantum_usecs;
 
        startInterruptTimer();

        TCB* main_thread = new TCB(0, RUNNING);
        //handle error

        thread_translation[0] = main_thread;

        current_thread = main_thread;

        //addToReadyQueue(main_thread);

        global_thread_count+=1;

        cout << "main thread created" << endl;

        return main_thread->getId();

}

int uthread_create(void* (*start_routine)(void*), void* arg)
{
        // Create a new thread and add it to the ready queue

        TCB* thread_instance = new TCB(global_thread_count, start_routine, arg, READY);

        addToReadyQueue(thread_instance);

        thread_translation[global_thread_count] = thread_instance;
        cout << "Thread " << thread_instance->getId() << " created" << endl;
        
        int returnVal = global_thread_count;
        global_thread_count++;

        return returnVal;
}

int uthread_join(int tid, void **retval)
{
        // If the thread specified by tid is already terminated, just return
        // If the thread specified by tid is still running, block until it terminates
        // Set *retval to be the result of thread if retval != nullptr
        if(thread_translation[tid]->getState() != BLOCK)
        {
                //put this thread on join to wait for the thread to finish
                join_queue_entry_t* john = new join_queue_entry_t();
                john->waiting_for_tid = tid;
                john->tcb = current_thread;
                john->tcb->setState(BLOCK);

                join_queue.push_back(john);
                uthread_yield();
                cout << "post first thread exit";
        }
        //when the thread is awoken it will return here
        //thread of interest should be terminated/blocked
        if(thread_translation[tid]->getState() == BLOCK)
        {
                //return 
                if(retval != nullptr)
                {
                        for (deque<finished_queue_entry_t*>::iterator iter = finish_queue.begin(); iter != finish_queue.end(); ++iter)
                        {
                                if (tid == (*iter)->tcb->getId())
                                {
                                        *retval = (*iter)->result;
                                        return 0;
                                }
                        }
                        //finished thread was not found
                        //it should have been in queue due to uthread_exit()
                        return -1;
                }
        }
        //joined thread should have been terminated
        //failed to join thread
        return -1;
}

int uthread_yield(void)
{
        disableInterrupts();
        switchThreads();
        return 0;
}

void uthread_exit(void *retval)
{
        // If this is the main thread, exit the program
        // Move any threads joined on this thread back to the ready queue
        // Move this thread to the finished queue
        int exiting_TID = uthread_self();
        
        //if main exit
        if(exiting_TID == 0)
        {
                exit(0);
                //is this exit enough?
        }
        else
        {
                //move this thread to finished deque
                finished_queue_entry_t* fini = new finished_queue_entry_t();
                cout << (*(unsigned long*)retval) << " retval" << endl;
                fini->result = retval;
                fini->tcb = current_thread;
                fini->tcb->setState(BLOCK);

                finish_queue.push_back(fini);
                //put main back on ready (using join queue)
                //iterate through join queue and find a joined thread

                // for (deque<join_queue_entry_t*>::iterator iter = join_queue.begin(); iter != join_queue.end(); ++iter){

                //         cout << (*iter)->tcb->getId() << "FOR LOOP CHECK" << endl;

                // }

                for (deque<join_queue_entry_t*>::iterator iter = join_queue.begin(); iter != join_queue.end(); ++iter)
                {
                        if (exiting_TID == (*iter)->waiting_for_tid)
                        {
                                cout << (*iter)->waiting_for_tid << " waiting for tid" << endl;
                                cout << exiting_TID << " exiting TID" << endl;
                                //ready the joined thread
                                ready_queue.push_back((*iter)->tcb);
                                //cout << "pushed to readyQ iterTcb:" << (*iter)->tcb->getId() << endl;
                                (*iter)->tcb->setState(READY);
                                //delete from join queue
                                join_queue.erase(iter);
                                uthread_yield();
                                return;
                        }
                }
        }

        cout <<"test"<<endl;
        //exit(0);
}

int uthread_suspend(int tid)
{
        // Move the thread specified by tid from whatever state it is
        // in to the block queue
        if(thread_translation[tid]->getState() == BLOCK)
        {
                //thread already blocked
                return -1;
        }
        else if(thread_translation[tid]->getState() == READY)
        {
                thread_translation[tid]->setState(BLOCK);
                block_queue.push_back(thread_translation[tid]);
                removeFromReadyQueue(tid);
                return 0;
        }
        else if(thread_translation[tid]->getState() == RUNNING)
        {
                thread_translation[tid]->setState(BLOCK);
                block_queue.push_back(thread_translation[tid]);
                removeFromReadyQueue(tid);
                uthread_yield();
                return 0;
        }
        //thread does not have a state
        return -1;
}

int uthread_resume(int tid)
{
        // Move the thread specified by tid back to the ready queue
        if(thread_translation[tid]->getState() == BLOCK)
        {
                thread_translation[tid]->setState(READY);
                addToReadyQueue(thread_translation[tid]);
                return 0;
        }
        else
        {
                //failed to resume
                return -1;
        }
        
}

int uthread_self()
{
        return current_thread->getId();
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
