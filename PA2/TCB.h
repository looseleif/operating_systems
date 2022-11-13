/*
 *
 */
#ifndef TCB_H
#define TCB_H

#include "uthread.h"
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>

extern void stub(void *(*start_routine)(void *), void *arg);

enum State {READY, RUNNING, BLOCK};
enum Priority {RED, ORANGE, GREEN};

/*
 * The thread
 */
class TCB
{

public:
	/**
	 * Constructor for TCB. Allocate a thread stack and setup the thread
	 * context to call the stub function
	 * @param tid id for the new thread
	 * @param f the thread function that get no args and return nothing
         * @param arg the thread function argument
	 * @param state current state for the new thread
	 */
	TCB(int tid, void *(*start_routine)(void* arg), void *arg, State state, Priority priority);
	
	/**
	 * thread d-tor
	 */
	~TCB();

	/**
	 * function to set the thread state
	 * @param state the new state for our thread
	 */
	void setState(State state);
	
	/**
	 * function that get the state of the thread
	 * @return the current state of the thread
	 */
	State getState() const;
	
	/**
	 * function that get the ID of the thread
	 * @return the ID of the thread 
	 */
	int getId() const;
	
		
	/**
	 * function to increase the quantum of the thread
	 */
	void increaseQuantum();
	
	/**
	 * function that get the quantum of the thread
	 * @return the current quantum of the thread
	 */
	int getQuantum() const;

	/**
	 * function that increments the thread's lock count
	 */
	void increaseLockCount();

	/**
	 * function that decrements the thread's lock count
	 */
	void decreaseLockCount();

	/**
	 * function that returns the number of locks held by this thread
	 */
	int getLockCount();

    /**
	 * function that returns a pointer to the thread's context storage location
         * @return zero on success, -1 on failure
	 */
	ucontext_t* getContext();

	/**
	 * function to set the thread priority
	 * @param priority the new priority for our thread
	 */
	void setPriority(Priority prio);
	
	/**
	 * function that gets the priorty of the thread
	 * @return the current priority of the thread
	 */
	Priority getPriority() const;



private:
	int _tid;               // The thread id number.
	int _quantum;           // The time interval, as explained in the pdf.
	State _state;           // The state of the thread
	int _lock_count;        // The number of locks held by the thread
	char* _stack;           // The thread's stack
	ucontext_t _context;    // The thread's saved context
	Priority _priority;
};


#endif /* TCB_H */
