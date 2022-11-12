#include "Lock.h"
#include "uthread_private.h"

// TODO
Lock::Lock()
{
    current_owner = nullptr;
    return_thread = nullptr;
    _is_sig = false;
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
    cout << "Unlocking... sig=" << _is_sig << endl;
    if(_is_sig)
    {
        disableInterrupts();
        cout << "signaled thread releasing lock, going back to signaler" << endl;
        _is_sig = false;
        current_owner = return_thread;
        switchToThread(return_thread);
        enableInterrupts();
    }
    else if(waiting_for_lock.empty())
    {
        //cout << "Lock queue empty" << endl;
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
    _is_sig = true;
    return_thread = running;
    cout << "Return thread set... switching to signaled thread." << endl;
    cout << _is_sig << endl;
    current_owner = tcb;
    switchToThread(tcb);

}

void Lock::_unlock()
{
    int calling_tid = running->getId();
    cout << "_Unlocking... sig=" << _is_sig << endl;
    if(_is_sig)
    {
        cout << "signaled thread releasing lock, going back to signaler" << endl;
        _is_sig = false;
        current_owner = return_thread;
        switchToThread(return_thread);

    }
    else if(waiting_for_lock.empty())
    {
        //cout << "Unlocking and blocking..." << endl;
        current_owner = nullptr;
        switchThreads();
    } else {
        cout << "Chaning lock ownership and blocking..." << endl;
        //current_owner->setState(READY);
        //addToReady(current_owner);
        current_owner = waiting_for_lock.front();
        waiting_for_lock.pop();
        switchToThread(current_owner);

    }

}