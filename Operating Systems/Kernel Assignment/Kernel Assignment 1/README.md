Operating Systems
=================

#### Kernel #1

##### This test assignment is evenly splited in our group.
##### Split:

* Yu Sun(sun812@usc.edu):					100/3			thread.c, sched.c     
* Heguang Liu(heguangl@usc.edu):			100/3			proc.c, kmain.c, mutex.c
* Tao Hu(taohu@usc.edu):					100/3			git, gradeline, test, shell,

Tips to test out kernel:

There is a global variable static int CURRENT_TEST is kmain.c

when you assign different mode to it, different test method get executed, the execute mode showed below:

	#define NORMAL_TEST                 0     /*The default run mode, create a process and exit*/
	#define DEADLOCK_TEST               1     /*test mode for deadlock with kmutex_lock, one thread hold the mutex and exit*/
	#define PRODUCER_CONSMUER_TEST      2     /*test mode for one producer vs one consumer*/
	#define DEADLOCK_NORMAL_TEST        3     /*test mode for deadlock with kmutex_lock, two threads try to lock another mutex without giving out current mutex*/
	#define KSHELL_TEST                 4     /*test mode for kshell, you can type in command like help in kshell*/
	#define READER_WRITER_TEST          5     /*test mode for reader,writer, we have 5 readers and 3 writers try to access the resources*/
	#define KILL_ALL_TEST               6     /*test mode for proc_kill_all()*/
	#define DEADLOCK_CANCELLABLE_TEST   7     /*test mode for deadlock with kmutex_lock_cancellable, two threads try to lock another mutex without giving out current mutex*/
	#define PROC_KILL_TEST              8     /*test mode for proc_kill, kill process 3 and 4 in current process*/
	#define PROC_EXIT_TEST              9     /*test mode for do_exit, just exit current process*/

Note:  you may want to turn the DRIVER to 1 in config.mk if you are not using our config.mk


