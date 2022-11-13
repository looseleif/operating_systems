CC = g++
CFLAGS = -g -lrt --std=c++14
DEPS = TCB.h uthread.h uthread_private.h Lock.h CondVar.h SpinLock.h
OBJ = TCB.o uthread.o Lock.o CondVar.o SpinLock.o
objects = uthread-sync-demo broadcastTest lockSpeedTest spinLockSpeedTest
all: $(objects)
MAIN_OBJ = originalMain.o
BROAD_OBJ = broadcastTestCase.o
LOCK_SP_OBJ = lockSpeedTest.o
SPIN_SP_OBJ = spinLockSpeedTest.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

uthread-sync-demo: $(OBJ) $(MAIN_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
broadcastTest: $(OBJ) $(BROAD_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
lockSpeedTest: $(OBJ) $(LOCK_SP_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
spinLockSpeedTest: $(OBJ) $(SPIN_SP_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	

.PHONY: clean

clean:
	rm -f *.o $(objects)
