#include "SpinLock.h"

// TODO
SpinLock::SpinLock()
{

}

void SpinLock::lock()
{
    //keep testing the atomic value
    while(atomic_value.test_and_set())
    {
        ;
    }
}

void SpinLock::unlock()
{
    //clear the atomic lock so that another thread can use it
    atomic_value.clear();
}