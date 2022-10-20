Programming Assignment 1
User Thread Library

Programmers:

Emir Sahbegovic sahbe001@umn.edu
Chase Anderson and08479@umn.edu

How to Compile:

once you have unzipped the tarball and you are inside the pa1 folder,
adjust the Makefile's line 4 entry and change the X of test_X to whichever test you'd like,
available tests include: "default" | "susRes" | "yield" | "extra1" | "extra2",
once you have changed the make file you can run make inside the pa1 folder

How to Run:

for "default" | "susRes" | "yield" your run conditions will look like similar,
./uthread-demo ### $$
### is number of points, $$ is number of threads

for "extra1" | "extra2" no extra input parameters are needed,
run and observe outputs to judge readiness

mind the value of MAX_THREAD_NUM, for that will dictate the maximum number of threads allowed,
a disregard of this number will cause a fault in the execution of your Programmers

Test Cases:

We didn’t have enough time to create a script which will run test cases automatically. 
We have screenshots which show what each test case output looks like. 
The test cases can be compiled and executed by editing the Makefile on line 4.

test_default, quantum==1000usec is used to check basic operation of context switching, init, and create.

test_susRes, quantum==1000000 is used to check the operation of suspend and resume functions. 
Notice that thread 1 switches to thread 3, rather than thread 2.

test_yield quantum==1000000usec is used to check the operation of yield. 
It uses suspend() and resume() to illustrate that thread 2 is not suspended immediately due to main yielding.

*, We also checked to make sure that the library wasn’t making too many threads.

Finally, we will mention our satisfactory test of get_quantum(int tid) and get_total_quantum(). 
Within each of our screenshots we have shown the printout statements of these functions; 
however, in the interest of cleanliness we have omitted these printouts for the published library.