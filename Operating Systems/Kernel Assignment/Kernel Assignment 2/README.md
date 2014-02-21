Operating Systems
=================

#### Kernel #2

##### This test assignment is evenly splited in our group.
##### Split:

* Yu Sun(sun812@usc.edu):  		        	100/3 		  vfs_syscall.c,vnode.c
* Heguang Liu(heguangl@usc.edu):		    100/3		  namev.c, vnode.c,vfs_syscall.c
* Tao Hu(taohu@usc.edu):			        100/3		  testcase, open.c,vfs_syscall.c

Tips to test out kernel:

There is a global variable static int CURRENT_TEST is kmain.c

when you assign different mode to it, different test method get executed, the execute mode showed below:


	#define KSHELL_TEST                 4     /*test mode for kshell, you can type in command like help in kshell*/
	#define VFS_TEST                    10    /*main test code for VFS*/

in VFS_TEST, we including following test cases:

 	vfstest_start();
    vfstest_stat();
    vfstest_mkdir();
    vfstest_chdir();
    vfstest_paths();
    vfstest_fd();
    vfstest_open();
    vfstest_read();
    vfstest_getdents();
    /*vfstest_infinite();*/ //this is a special test case, this lead to halt 

Note:  you may want to turn the DRIVER to 1 and  in config.mk if you are not using our config.mk