Operating Systems
=================

#### Kernel Programming Assignments
###### Bill Chen
http://merlot.usc.edu/cs402-s13/

##### Kernel Programming Assignments
* Tom Doeppner’s weenix source and binary code
	- provided as weenix-assignment-1.0.2.tar.gz
	- incomplete
	- contains code like:
		* NOT_YET_IMPLEMENTED("PROCS: bootstrap");
			- assignment name ---|        |
			- function name --------------|
		* your job is to implement these functions by replacing these lines with your code
		* please replace them in-place
	- to look for such code:


			grep PROCS: kernel/*.c
       		grep PROCS: kernel/*/*.c
       		grep PROCS: kernel/*/*/*.c
       		grep PROCS: kernel/*/*/*/*.c


##### Download and Setup
	% gunzip -c weenix-assignment-1.0.2.tar.gz | \
        tar xvf -
	% cd weenix-assignment-1.0.2/weenix
	% make clean
	% make
	% ./weenix -n
* if all goes well, you should see tons of stuff fly by and the following at the bottom of the console:


		Not yet implemented: PROCS: bootstrap, file main/kmain.c, line 127
		panic in main/kmain.c:129 bootstrap(): weenix returned to bootstrap()!!! BAD!!!
		Kernel Halting.


##### Documentation
* The weenix documentation is in doc/latex/documentation.pdf
	- introduces weenix to you
	- detailed instructions on assignments
	- you must read it thoroughly

* We are doing three of the assignments
	- Processes and Threads (PROCS)
	- Virtual File System (VFS)
	- Virtual Memory (VM) - extra credit assignment

* We are not doing two of the assignments
	- Drivers (DRIVERS)
	- System V File System (S5FS)
	- these are done for you and they are compiled and provided as libraries
		* kernel/libdrivers.a and kernel/libs5fs.a

##### Compilation and Configuration
* Config.mk controls what gets compiles and configured into the kernel
	- for PROCS, use the original Config.mk
	- for VFS, set DRIVERS and VFS to 1
	- for VM, set DRIVERS, VFS, S5FS, VM, and DYNAMIC to 1

* Modify Config.mk first, then do:


		% make clean
    	% make
    	% ./weenix -n