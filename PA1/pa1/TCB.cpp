#include "TCB.h"

// constructor for non intial threads, for implementation of multithreading
TCB::TCB(int tid, void *(*start_routine)(void* arg), void *arg, State state)
{
    this->_tid = tid;

    getcontext(&(this->_context));

    this->_context.uc_stack.ss_sp = new char[STACK_SIZE];
    this->_context.uc_stack.ss_size = STACK_SIZE;
    this->_context.uc_stack.ss_flags = 0;

    this->_quantum = 0;
    this->_state = state;

    // creates context with reference to stub, always the function we wish to multithread
    makecontext(&(this->_context), (void(*)())stub, 2, start_routine, arg);

}


// constructor for initial thread, prior to multithreading, usually main
TCB::TCB(int tid, State state)
{
    
    if(getcontext(&(this->_context))==0){

        this->_tid = tid;

    } else {

        this->_tid = -1;

    }

    this->_quantum = 1;
    this->_state = state;

}

TCB::~TCB()
{
    // basic clean of TCB
    delete &(this->_context);

}

void TCB::setState(State state)
{

    this->_state = state;

}

State TCB::getState() const
{

    return this->_state;

}

int TCB::getId() const
{

    return this->_tid;

}

void TCB::increaseQuantum()
{

    this->_quantum += 1;

}

int TCB::getQuantum() const
{
    
    return this->_quantum;

}

int TCB::saveContext()
{
    return getcontext(&(this->_context));
}

// loading of caller TCB both, either initial load, or resume of context
void TCB::loadContext()
{
    this->increaseQuantum();
    setcontext(&(this->_context));
}
