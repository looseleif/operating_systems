#include "uthread.h"
#include "TCB.h"
#include <cassert>
#include <deque>
#include <map>

using namespace std;

// finished queue entry type
typedef struct finished_queue_entry {
  TCB *tcb;             // Pointer to TCB
  void *result;         // Pointer to thread result (output)
} finished_queue_entry_t;

// join queue entry type
typedef struct join_queue_entry {
  TCB *tcb;             // Pointer to TCB
  int waiting_for_tid;  // TID this thread is waiting on
} join_queue_entry_t;

// Queues
static deque<TCB*> ready_queue;
static deque<TCB*> block_queue; //may not be necessary?
static deque<finished_queue_entry_t*> finish_queue;
static deque<join_queue_entry_t*> join_queue;

// Thread Management --------------------------------------------------------

// used for run-time reference to threads during execution
static int global_thread_count = 0;

static TCB* current_thread;
static TCB* thread_translation[MAX_THREAD_NUM];

// Interrupt Management --------------------------------------------------------

// block signals from firing timer interrupt
static struct sigaction _sa;

static bool interrupts_enabled = true;

static void time_int(int signal_num){

        uthread_yield();
        return;

}

// ignore signals to disable timer interrupt
static void disableInterrupts()
{
        assert(interrupts_enabled);
	_sa.sa_handler = SIG_IGN;
        if (sigaction(SIGVTALRM, &_sa, NULL) == -1) {
        printf("error with: sigaction\n");
        exit(EXIT_FAILURE);
        }
        interrupts_enabled = false;
}

// refresh signals to enable timer interrupt
static void enableInterrupts()
{
        assert(!interrupts_enabled);
        interrupts_enabled = true;
        _sa.sa_handler = time_int;
        if (sigaction(SIGVTALRM, &_sa, NULL) == -1) {
        printf("error with: sigaction\n");
        exit(EXIT_FAILURE);
        }
}

// Queue Management ------------------------------------------------------------

// add TCB to the back of the ready queue
void addToReadyQueue(TCB *tcb)
{
        ready_queue.push_back(tcb);
}

// removes and returns the first TCB on the ready queue
// NOTE: Assumes at least one thread on the ready queue
TCB* popFromReadyQueue()
{
        assert(!ready_queue.empty());

        TCB *ready_queue_head = ready_queue.front();
        ready_queue.pop_front();
        return ready_queue_head;
}

// removes the thread specified by the TID provided from the ready queue
// returns 0 on success, and -1 on failure (thread not in ready queue)
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

        // thread not found
        return -1;
}

// Helper functions ------------------------------------------------------------

// switch to the next ready thread
static void switchThreads()
{
    // getcontext() will "return twice" - Need to differentiate between the two
    int ret_val = current_thread->saveContext();

    // this is the first return from getcontext (switching threads)
    if(current_thread->getState() != BLOCK && current_thread->getState() != FINISHED)
    {
        addToReadyQueue(current_thread);
        current_thread->setState(READY);
    }
    current_thread = popFromReadyQueue();
    current_thread->setState(RUNNING);
    cout << "switching to thread " << current_thread->getId() << endl;
    current_thread->loadContext();
    enableInterrupts();
}


// Library functions -----------------------------------------------------------

// starting point for thread. calls top-level thread function
// enables interupts as we enter, in prep for switch
void stub(void *(*start_routine)(void *), void *arg)
{
        enableInterrupts();
        uthread_exit(start_routine(arg));
}

// initialize our space for multithreaded applications
// init and set timer based interrupts
int uthread_init(int quantum_usecs)
{
        // necessary struct for configuring timer interrupts
        struct itimerval tv;

        // clear any previous signal sets
        sigemptyset(&_sa.sa_mask);
        _sa.sa_flags = 0;
        _sa.sa_handler = time_int;
        if (sigaction(SIGVTALRM, &_sa, NULL) == -1) {
        printf("error with: sigaction\n");
        exit(EXIT_FAILURE);
        }

        // sets timer to given quantum
        tv.it_value.tv_sec = 0;
        tv.it_value.tv_usec = quantum_usecs;
        tv.it_interval.tv_sec = 0;
        tv.it_interval.tv_usec = quantum_usecs;
 
        // adds time_int handler associated with SIGVTALRM
        signal(SIGVTALRM, time_int);

        // starts interrupt timer
        setitimer(ITIMER_VIRTUAL, &tv, NULL);

        TCB* main_thread = new TCB(0, RUNNING);
        if(main_thread->getId()==-1){
                cerr << "error: Failure to create main thread" << endl;
                exit(-1);
        }

        // for purposes of cleanliness, we have assigned main thread to 0
        thread_translation[0] = main_thread;
        current_thread = main_thread;
        global_thread_count+=1;

        return main_thread->getId();

}

// create a new thread and add it to the ready queue
int uthread_create(void* (*start_routine)(void*), void* arg)
{
        if(global_thread_count==MAX_THREAD_NUM){
                cerr << "error: Too many threads created!" << endl;
                exit(-1);
        }

        // creates new thread instance
        TCB* thread_instance = new TCB(global_thread_count, start_routine, arg, READY);

        addToReadyQueue(thread_instance);

        thread_translation[global_thread_count] = thread_instance;        
        int returnVal = global_thread_count;
        global_thread_count++;

        return returnVal;
}

int uthread_join(int tid, void **retval)
{
        // if the thread specified by tid is already terminated, just return
        // if the thread specified by tid is still running, block until it terminates
        // set *retval to be the result of thread if retval != nullptr
        if(thread_translation[tid]->getState() != FINISHED)
        {
                // put this thread on join to wait for the thread to finish
                join_queue_entry_t* john = new join_queue_entry_t();
                john->waiting_for_tid = tid;
                john->tcb = current_thread;
                john->tcb->setState(BLOCK);

                join_queue.push_back(john);
                uthread_yield();
        }
        
        // when the thread is awoken it will return here
        // thread of interest should be terminated/blocked
        if(thread_translation[tid]->getState() == FINISHED)
        {
                // return 
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

                        // finished thread was not found
                        // it should have been in queue due to uthread_exit()
                        return -1;
                }
        }

        // joined thread should have been terminated
        // failed to join thread
        return -1;
}

// used to properly change context switch between threads
int uthread_yield(void)
{
        disableInterrupts();
        switchThreads();
        return 0;
}

void uthread_exit(void *retval)
{
        // if this is the main thread, exit the program
        // move any threads joined on this thread back to the ready queue
        // move this thread to the finished queue
        int exiting_TID = uthread_self();
        
        // if main, exit
        if(exiting_TID == 0)
        {
                exit(0);
        }
        else
        {
                //move this thread to finished deque
                finished_queue_entry_t* fini = new finished_queue_entry_t();
                fini->result = retval;
                fini->tcb = current_thread;
                fini->tcb->setState(FINISHED);

                finish_queue.push_back(fini);

                //put main back on ready (using join queue)
                //iterate through join queue and find a joined thread
                for (deque<join_queue_entry_t*>::iterator iter = join_queue.begin(); iter != join_queue.end(); ++iter)
                {
                        if (exiting_TID == (*iter)->waiting_for_tid)
                        {
                                cout << (*iter)->waiting_for_tid << " waiting for tid" << endl;
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
                uthread_yield();
        }

}

// checks thread state and either places on to BLOCK queue or has no action
int uthread_suspend(int tid)
{
        // move the thread specified by tid from whatever state it is
        // in to the block queue
        if(thread_translation[tid]->getState() == BLOCK)
        {
                // thread already blocked
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

        // thread does not have a state
        return -1;
}

// will unblock specified thread, allowed to resume RR scheduling
int uthread_resume(int tid)
{
        // move the thread specified by tid back to the ready queue
        if(thread_translation[tid]->getState() == BLOCK)
        {
                thread_translation[tid]->setState(READY);
                addToReadyQueue(thread_translation[tid]);
                return 0;
        }
        else
        {
                // failed to resume
                return -1;
        }
        
}

// returns the TID of caller thread
int uthread_self()
{
        return current_thread->getId();
}

// total number of quantums elasped over all threads at time of call
int uthread_get_total_quantums()
{
        int totalQuant = 0;

        for(int i = 0; i<global_thread_count; i++){

                totalQuant += thread_translation[i]->getQuantum();

        }
        return totalQuant;
}

// total number of quantums elasped so far for specified thread
int uthread_get_quantums(int tid)
{
        return thread_translation[tid]->getQuantum();
}
