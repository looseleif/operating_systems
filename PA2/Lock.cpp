#include "Lock.h"
#include "uthread_private.h"

// TODO
Lock::Lock()
{
    current_owner = nullptr;
    return_thread = nullptr;
    is_sig = false;
}

void Lock::lock()
{
    //check availability

    //no owner
    int calling_tid = running->getId();
    if(current_owner == nullptr)
    {
        //aquire lock
        current_owner = running;
        return;
    }
    //another thread has this lock
    else
    {
        //add to waiting queue and suspend this thread
        disableInterrupts();

        waiting_for_lock.push(running);
        running->setState(BLOCK);

        switchThreads();
        enableInterrupts();
    }

    return;

}

void Lock::unlock()
{

    int calling_tid = running->getId();
    if(is_sig)
    {
        is_sig = !is_sig;
        current_owner = return_thread;
        switchToThread(return_thread);
    }
    else if(waiting_for_lock.empty()){

        current_owner = nullptr;
        return;

    } else {
        disableInterrupts();

        current_owner->setState(READY);
        addToReady(current_owner);
        current_owner = waiting_for_lock.front();
        waiting_for_lock.pop();
        switchToThread(current_owner);
        enableInterrupts();
    }

}

void Lock::_signal(TCB *tcb)
{

    is_sig = true;
    return_thread = running;
    current_owner = tcb;
    switchToThread(tcb);

}

void Lock::_unlock()
{

    int calling_tid = running->getId();

    if(is_sig)
    {
        is_sig = !is_sig;
        current_owner = return_thread;
        switchToThread(return_thread);
    }
    else if(waiting_for_lock.empty())
    {
        current_owner = nullptr;
        return;
    } else {

        //current_owner->setState(READY);
        //addToReady(current_owner);
        current_owner = waiting_for_lock.front();
        waiting_for_lock.pop();
        switchToThread(current_owner);

    }

}