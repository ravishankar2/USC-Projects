Operating Systems
=================

#### Kernel #3

#### This test assignment is evenly splited in our group.
##### Split:

* Yu Sun(sun812@usc.edu):			        100/3 		pagefault.c mmap.c access.c brk.c proc.c
* Heguang Liu(heguangl@usc.edu):		    100/3		vammap.c anon.c vnode.c fork.c kthread.c
* Tao Hu(taohu@usc.edu):			        100/3		shadow.c syscall.c kmain.c syscall.c

Bug report:

We have implemented all 43 required functions for this assignment, the S5FS works perfectly and VM is partly working. We are able to enter userland and able to handle pagetable refresh, but there is a known bug when the system try to access the Page Number 0. As we cannot find corresponding vmarea, our code proc_kill the current process as required. 


Extra info:

We've been suffered in kernel3 for half a month. We have added all the KASSERT in our dbg and you can see this when running our two tests. We've tried our best and life is not easy. If you noticed the KASSERT statement in your output, could you please consider to give us the points? :-)


Tips to test out kernel:

There is a global variable static int CURRENT_TEST is kmain.c. When you assign different mode to it, different test method get executed, the execute mode showed below:

	#define KSHELL_TEST                 4         /*implemented s5fs test, work perfectly*/
	#define VM_TEST                     11        /*implemented vm test, panic in access page 0*/

Submitting files:

kmain.c kmutex.c kthread.c proc.c sched.c namev.c

open.c vfs_syscall.c vnode.c

access.c syscall.c fork.c kthread.c anon.c brk.c mmap.c pagefault.c shadow.c vmmap.c

Config.mk  vm_README.txt




