.PHONY: all clean all_kernel all_user clean_kernel clean_user nyi
all: all_kernel all_user

all_kernel:
	@ cd kernel && $(MAKE) all

all_user:
	@ cd user && $(MAKE) all

clean: clean_kernel clean_user

clean_kernel:
	@ cd kernel && $(MAKE) clean

clean_user:
	@ cd user && $(MAKE) clean

nyi:
	@ cd kernel && $(MAKE) nyi

procs-submit:
	tar cvzf procs-submit.tar.gz \
		Config.mk \
		procs-README.txt \
		kernel/main/kmain.c \
		kernel/proc/kmutex.c \
		kernel/proc/kthread.c \
		kernel/proc/proc.c \
		kernel/proc/sched.c

vfs-submit:
	tar cvzf vfs-submit.tar.gz \
		Config.mk \
		vfs-README.txt \
		kernel/main/kmain.c \
		kernel/proc/kmutex.c \
		kernel/proc/kthread.c \
		kernel/proc/proc.c \
		kernel/proc/sched.c \
		kernel/fs/namev.c \
		kernel/fs/open.c \
		kernel/fs/vfs_syscall.c \
		kernel/fs/vnode.c

vm-submit:
	tar cvzf vm-submit.tar.gz \
		Config.mk \
		vm-README.txt \
		kernel/main/kmain.c \
		kernel/proc/kmutex.c \
		kernel/proc/kthread.c \
		kernel/proc/proc.c \
		kernel/proc/sched.c \
		kernel/fs/namev.c \
		kernel/fs/open.c \
		kernel/fs/vfs_syscall.c \
		kernel/fs/vnode.c \
		kernel/api/access.c \
		kernel/api/syscall.c \
		kernel/fs/vnode.c \
		kernel/proc/fork.c \
		kernel/proc/kthread.c \
		kernel/vm/anon.c \
		kernel/vm/brk.c \
		kernel/vm/mmap.c \
		kernel/vm/pagefault.c \
		kernel/vm/shadow.c \
		kernel/vm/vmmap.c
