#include "TCB.h"

TCB::TCB(int tid, void *(*start_routine)(void* arg), void *arg, State state)
{
    this->_tid = tid;

    getcontext(&(this->_context));

    this->_context.uc_stack.ss_sp = new char[STACK_SIZE];
    this->_context.uc_stack.ss_size = STACK_SIZE;
    this->_context.uc_stack.ss_flags = 0;

    // check piazza
    //sigaddset(&(this->_context.uc_sigmask), SIGVTALRM);

    this->_state = state;

    makecontext(&(this->_context), (void(*)())stub, 2, start_routine, arg);

}

TCB::TCB(int tid, State state)
{
    
    if(getcontext(&(this->_context))==0){

        this->_tid = tid;

    } else {

        this->_tid = -1;

    }

    this->_state = state;

}

TCB::~TCB()
{

    //delete &(this->_context);

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

    this->_quantum += 100;

}

int TCB::getQuantum() const
{
    
    return this->_quantum;

}

int TCB::saveContext()
{
    return getcontext(&(this->_context));
}

void TCB::loadContext()
{
    setcontext(&(this->_context));
}
