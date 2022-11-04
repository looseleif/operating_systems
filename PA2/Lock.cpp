#include "Lock.h"
#include "uthread_private.h"

// TODO
Lock::Lock()
{
    current_owner = nullptr;
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

    if(waiting_for_lock.empty()){

        current_owner = nullptr;
        return;

    } else {
        disableInterrupts();

        current_owner->setState(READY);
        addToReady(current_owner);
        current_owner = waiting_for_lock.front();

        switchToThread(current_owner);
        enableInterrupts();
    }

}