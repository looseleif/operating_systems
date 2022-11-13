#include "CondVar.h"
#include "uthread_private.h"

// TODO
CondVar::CondVar()
{

}

void CondVar::wait(Lock &lock)
{
    
    _heldLock = &lock;
    disableInterrupts();
    waiting_for_signal.push(running);
    _heldLock->_unlock();
    enableInterrupts();

}

void CondVar::signal()
{
    
    if(waiting_for_signal.empty()){
        
        //disableInterrupts();
        //_heldLock._unlock();
        //enableInterrupts();

    } else {
    
    disableInterrupts();
    //cout << "Signalling..." << endl;
    TCB* retrieved = waiting_for_signal.front();
    waiting_for_signal.pop();
    _heldLock->_signal(retrieved);

    enableInterrupts();

    }
}

void CondVar::broadcast()
{
    
    if(waiting_for_signal.empty()){
        
        //disableInterrupts();
        //_heldLock._unlock();
        //enableInterrupts();

    } else {
    
    disableInterrupts();
    //cout << "Broadcasting..." << endl;
    while(!(waiting_for_signal.empty()))
    {
    	TCB* retrieved = waiting_for_signal.front();
    	waiting_for_signal.pop();
    	_heldLock->_signal(retrieved);
    }

    enableInterrupts();

    }
}
