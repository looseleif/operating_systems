#include "Lock.h"
#include "uthread_private.h"

// TODO
Lock::Lock()
{
    current_owner = nullptr;
    return_thread = nullptr;
    _is_sig = false;
    prev_prior = -1;
    prior_inv_hold = nullptr;
    priorDiff = 0;
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

        // --- ATTEMPTED SOLUTION FOR PRIORITY INVERSION

        // int checkDiff = running->getPriority()-current_owner->getPriority();

        // if(checkDiff>0){

        //     cout << "assuming higher priority with TID of " << current_owner->getId() << endl;

        //     priorDiff = checkDiff;
        //     for(int i = 0; i<(priorDiff); i++){
        //     uthread_increase_priority(current_owner->getId());
        //     }

        // } else {

        //     checkDiff = 0;

        // }

        // ---

        waiting_for_lock.push(running);
        running->setState(BLOCK);
        
        enableInterrupts();
        uthread_yield();
        
    }

    return;

}

void Lock::unlock()
{
    int calling_tid = running->getId();
    //cout << "Unlocking... sig=" << _is_sig << endl;
    if(_is_sig)
    {
        disableInterrupts();
        //cout << "signaled thread releasing lock, going back to signaler" << endl;
        _is_sig = false;
        addToReady(current_owner);
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

        // --- ATTEMPTED SOLUTION FOR PRIORITY INVERSION

        // if(priorDiff>0){

        //     cout << "returning to original priority" << endl;

        //     for(int i = 0; i<(priorDiff); i++){
        //     uthread_decrease_priority(current_owner->getId());
        //     }

        //     priorDiff = 0;

        // }

        // ---

        current_owner->setState(READY);
        addToReady(current_owner);
        current_owner = waiting_for_lock.front();
        waiting_for_lock.pop();
        //switchToThread(current_owner);
        enableInterrupts();
        uthread_yield();
    }

}

void Lock::_signal(TCB *tcb)
{
    _is_sig = true;
    return_thread = running;
    //cout << "Return thread set... switching to signaled thread." << endl;
    //cout << _is_sig << endl;
    current_owner = tcb;
    switchToThread(tcb);

}

void Lock::_unlock()
{
    int calling_tid = running->getId();
    //cout << "_Unlocking... sig=" << _is_sig << endl;
    if(_is_sig)
    {
        disableInterrupts();
        //cout << "signaled thread releasing lock, going back to signaler (SPECIAL)" << endl;
        _is_sig = false;
        addToReady(current_owner);
        current_owner = return_thread;
        switchToThread(return_thread);
        enableInterrupts();

    }
    else if(waiting_for_lock.empty())
    {
        //cout << "Unlocking and blocking..." << endl;
        current_owner = nullptr;
        switchThreads();
    } else {
        //cout << "Chaning lock ownership and blocking..." << endl;
        //current_owner->setState(READY);
        //addToReady(current_owner);
        current_owner = waiting_for_lock.front();
        waiting_for_lock.pop();
        switchToThread(current_owner);

    }

}
