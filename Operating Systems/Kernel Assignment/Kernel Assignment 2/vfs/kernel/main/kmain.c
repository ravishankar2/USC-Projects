/*
 The goal of bootstrap()is to set up the first kernel thread and process which should start executing the idleproc_run()function. 
 This should be a piece of cake once you've implemented threads and the scheduler. 

 idleproc_run()performs further initialization 
 and starts the init process using initproc_create(), which starts executing in initproc_run(). All of your test code should be 
 in initproc_run(). When your operating system is nearly complete, it will execute a binary program in user mode, but for now, 
 be content to put together your own testing system for threads and processes. Once you have implemented terminal drivers, you 
 can test here by setting up the kernel shell from test/kshell/.
*/
#include "types.h"
#include "globals.h"
#include "kernel.h"

#include "util/gdb.h"
#include "util/init.h"
#include "util/debug.h"
#include "util/string.h"
#include "util/printf.h"

#include "mm/mm.h"
#include "mm/page.h"
#include "mm/pagetable.h"
#include "mm/pframe.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "main/acpi.h"
#include "main/apic.h"
#include "main/interrupt.h"
#include "main/cpuid.h"
#include "main/gdt.h"

#include "proc/sched.h"
#include "proc/proc.h"
#include "proc/kthread.h"

#include "drivers/dev.h"
#include "drivers/blockdev.h"
#include "drivers/tty/virtterm.h"

#include "api/exec.h"
#include "api/syscall.h"

#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/fcntl.h"
#include "fs/stat.h"

#include "test/kshell/kshell.h"

GDB_DEFINE_HOOK(boot)
GDB_DEFINE_HOOK(initialized)
GDB_DEFINE_HOOK(shutdown)

static void      *bootstrap(int arg1, void *arg2);
static void      *idleproc_run(int arg1, void *arg2);
static kthread_t *initproc_create(void);
static void      *initproc_run(int arg1, void *arg2);
static void       hard_shutdown(void);
static void      *test(int arg1, void *arg2);

static context_t bootstrap_context;

#define NORMAL_TEST                 0
#define DEADLOCK_TEST               1
#define PRODUCER_CONSMUER_TEST      2
#define DEADLOCK_NORMAL_TEST        3
#define KSHELL_TEST                 4
#define READER_WRITER_TEST          5
#define KILL_ALL_TEST               6
#define DEADLOCK_CANCELLABLE_TEST   7
#define PROC_KILL_TEST              8
#define PROC_EXIT_TEST              9

#define VFS_TEST                    10

static int CURRENT_TEST = VFS_TEST;

/**
 * This is the first real C function ever called. It performs a lot off
 * hardware-specific initialization, then creates a pseudo-context to
 * execute the bootstrap function in.
 */
void
kmain()
{
        GDB_CALL_HOOK(boot);

        dbg_init();
        dbgq(DBG_CORE, "Kernel binary:\n");
        dbgq(DBG_CORE, "  text: 0x%p-0x%p\n", &kernel_start_text, &kernel_end_text);
        dbgq(DBG_CORE, "  data: 0x%p-0x%p\n", &kernel_start_data, &kernel_end_data);
        dbgq(DBG_CORE, "  bss:  0x%p-0x%p\n", &kernel_start_bss, &kernel_end_bss);

        page_init();

        pt_init();
        slab_init();
        pframe_init();

        acpi_init();
        apic_init();
        intr_init();

        gdt_init();

        /* initialize slab allocators */
#ifdef __VM__
        anon_init();
        shadow_init();
#endif
        vmmap_init();
        proc_init();
        kthread_init();

#ifdef __DRIVERS__
        bytedev_init();
        blockdev_init();
#endif

        void *bstack = page_alloc();
        pagedir_t *bpdir = pt_get();
        KASSERT(NULL != bstack && "Ran out of memory while booting.");
        context_setup(&bootstrap_context, bootstrap, 0, NULL, bstack, PAGE_SIZE, bpdir);
        context_make_active(&bootstrap_context);

        panic("\nReturned to kmain()!!!\n");
}

/**
 * This function is called from kmain, however it is not running in a
 * thread context yet. It should create the idle process which will
 * start executing idleproc_run() in a real thread context.  To start
 * executing in the new process's context call context_make_active(),
 * passing in the appropriate context. This function should _NOT_
 * return.
 *
 * Note: Don't forget to set curproc and curthr appropriately.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
static void *
bootstrap(int arg1, void *arg2)
{
        /*--taohu--------dbg----------------*/
        dbg(DBG_CORE,"Enter bootstrap()\n");

        /* necessary to finalize page table information */
        pt_template_init();

        /* ---------------------heguang-------------------- */
        proc_t *idle_process=proc_create("idle_proc");
        /*--taohu--------Gradeline requirement----------------*/
        KASSERT(NULL != idle_process); /* make sure that the "idle" process has
been created successfully */
        KASSERT(PID_IDLE==idle_process->p_pid);/* make sure that what has been
created is the "idle" process */
         /*--taohu-------------------------------------------*/
        
        kthread_t *idle_thread=kthread_create(idle_process,idleproc_run,0,NULL);
        /*--taohu--------Gradeline requirement----------------*/
        KASSERT(NULL != idle_thread); /* make sure that the thread for the "idle"
process has been created successfully */
         /*--taohu-------------------------------------------*/

        curproc=idle_process;
        curthr=idle_thread;

        /*curproc->p_state=PROC_RUNNING;*/
        curthr->kt_state=KT_RUN;
        context_make_active(&curthr->kt_ctx);

        /*--taohu--------dbg----------------*/
        dbg(DBG_CORE,"Leave bootstrap()\n");

        /* ---------------------heguang-------------------- */
       panic("weenix returned to bootstrap()!!! BAD!!!\n");
        return NULL;
}

/**
 * Once we're inside of idleproc_run(), we are executing in the context of the
 * first process-- a real context, so we can finally begin running
 * meaningful code.
 *
 * This is the body of process 0. It should initialize all that we didn't
 * already initialize in kmain(), launch the init process (initproc_run),
 * wait for the init process to exit, then halt the machine.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
static void *
idleproc_run(int arg1, void *arg2)
{
        /*--taohu--------dbg----------------*/
        dbg(DBG_CORE,"Enter idleproc_run()\n");

        int status;
        pid_t child;

        /* create init proc */
        kthread_t *initthr = initproc_create();

        init_call_all();
        GDB_CALL_HOOK(initialized);

        /* Create other kernel threads (in order) */

#ifdef __VFS__
        /* Once you have VFS remember to set the current working directory
         * of the idle and init processes */
        curproc->p_cwd = vfs_root_vn;
        initthr->kt_proc->p_cwd = vfs_root_vn;
        vref(vfs_root_vn);

        dbg(DBG_VFS,"##########VFS: Enter do_mkdir/dev\n");
        do_mkdir("/dev");
        dbg(DBG_VFS,"##########VFS: Enter do_mknod/dev/null\n");
        do_mknod("/dev/null",S_IFCHR, MKDEVID(1,0));
        dbg(DBG_VFS,"##########VFS: Enter do_mknod/dev/zero\n");
        do_mknod("/dev/zero",S_IFCHR, MKDEVID(1,1));
        dbg(DBG_VFS,"##########VFS: Enter do_mknod/dev/tty0\n");
        do_mknod("/dev/tty0",S_IFCHR, MKDEVID(2,0));
        /* Here you need to make the null, zero, and tty devices using mknod */
        /* You can't do this until you have VFS, check the include/drivers/dev.h
         * file for macros with the device ID's you will need to pass to mknod */
#endif

        /* Finally, enable interrupts (we want to make sure interrupts
         * are enabled AFTER all drivers are initialized) */
        intr_enable();

        /* Run initproc */
        sched_make_runnable(initthr);
        /* Now wait for it */
        child = do_waitpid(-1, 0, &status);
        KASSERT(PID_INIT == child);

#ifdef __MTP__
        kthread_reapd_shutdown();
#endif


#ifdef __VFS__
        /* Shutdown the vfs: */
        dbg_print("weenix: vfs shutdown...\n");
        vput(curproc->p_cwd);
        if (vfs_shutdown())
                panic("vfs shutdown FAILED!!\n");

#endif

        /* Shutdown the pframe system */
#ifdef __S5FS__
        pframe_shutdown();
#endif

        dbg_print("\nweenix: halted cleanly!\n");
        GDB_CALL_HOOK(shutdown);
        hard_shutdown();
       /*--taohu--------dbg----------------*/
        dbg(DBG_CORE,"Enter idleproc_run()\n");
        return NULL;
}

/**
 * This function, called by the idle process (within 'idleproc_run'), creates the
 * process commonly refered to as the "init" process, which should have PID 1.
 *
 * The init process should contain a thread which begins execution in
 * initproc_run().
 *
 * @return a pointer to a newly created thread which will execute
 * initproc_run when it begins executing
 */
static kthread_t *
initproc_create(void)
{
    dbg(DBG_CORE,"Enter initproc_create()\n");
    proc_t *init_process=proc_create("init_process");
    KASSERT(NULL != init_process);
    KASSERT(PID_INIT==init_process->p_pid);
    kthread_t *init_thread=kthread_create(init_process,initproc_run,0,NULL);    
    KASSERT(NULL != init_thread);
    return init_thread;
    /* ---------------------heguang-------------------- */
     dbg(DBG_CORE,"Leave initproc_create()\n");
}

/**
 * The init thread's function changes depending on how far along your Weenix is
 * developed. Before VM/FI, you'll probably just want to have this run whatever
 * tests you've written (possibly in a new process). After VM/FI, you'll just
 * exec "/bin/init".
 *
 * Both arguments are unused.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
 /* Yu Sun Code Start */
/* ----------------for test------------------- */
static void       normal_test();
static void       deadlock_test();
static void       producer_consmuser_test();
static void       deadlock_run();
static void       kshell_test();
static void      *kshell_run(int arg1, void *arg2);
kmutex_t          mtx, pc_mutex;
kmutex_t          dead_mtx1,dead_mtx2;
static void      *deadlock1_run(int arg1, void *arg2);
static void      *deadlock2_run(int arg1, void *arg2);
static void      *deadlock(int arg1, void *arg2);
int               share_resource;
static void      *producer(int arg1, void *arg2);
static void      *consumer(int arg1, void *arg2);
ktqueue_t         kt_wproq, kt_wconq;
static void       reader_writer_test();
static void      *reader(int arg1, void *arg2);
static void      *writer(int arg1, void *arg2);
kmutex_t          wr_mutex;
ktqueue_t         kt_readq, kt_writeq;
int               reading, writing;
static void       deadlock_cancellable_run();
static void      *deadlock3_run(int arg1, void *arg2);
static void      *deadlock4_run(int arg1, void *arg2);
kmutex_t          dead_mtx3,dead_mtx4;
static void       proc_kill_run();
static void       killall_normal_run();
static void       proc_exit_run();
static void      *exit_test(int arg1, void *arg2);
static void      *normal_test_run(int arg1, void *arg2);

static void       vfs_test_run();

static void *
initproc_run(int arg1, void *arg2)
{
     /*--taohu--------dbg----------------*/
     dbg(DBG_CORE,"Enter initproc_run()\n");

     switch(CURRENT_TEST) {
        case NORMAL_TEST:
            normal_test();
            break;
        case DEADLOCK_TEST:
            deadlock_test();
            break;
        case PRODUCER_CONSMUER_TEST:
            producer_consmuser_test();
            break;
        case DEADLOCK_NORMAL_TEST:
            deadlock_run();
            break;
        case KSHELL_TEST:
            kshell_test();
            break;
        case READER_WRITER_TEST:
            reader_writer_test();
            break;
        case DEADLOCK_CANCELLABLE_TEST:
            deadlock_cancellable_run();
            break;
        case PROC_KILL_TEST:
            proc_kill_run();
            break;
        case KILL_ALL_TEST:
            killall_normal_run();
            break;
        case PROC_EXIT_TEST:
            proc_exit_run();
            break;
        case VFS_TEST:
            vfs_test_run();
            break;
     }

    int status;
    while(!list_empty(&curproc->p_children))
    {
        int cpid = do_waitpid(-1,0,&status);
        KASSERT(status==0);
        dbg_print("process %d return.\n",cpid);
    }

     /*--taohu--------dbg----------------*/
     dbg(DBG_CORE,"Leave initproc_run()\n");
    return NULL;
}

static void 
proc_exit_run()
{
    dbg(DBG_CORE,"Test PROC_EXIT_TEST\n");
    pid_t child=0;
    int status=0;
    proc_t * process=proc_create("test_process");
    kthread_t* thread=kthread_create(process,exit_test,0,NULL);
    sched_make_runnable(thread);

    child=do_waitpid(-1,0,&status);
    KASSERT(status==0);
    dbg_print("process %d return.\n",(int)child);
}

static void *
exit_test(int arg1, void *arg2)
{
    dbg(DBG_CORE,"Proc%d do exiting...\n", curproc -> p_pid);
    do_exit(0);
    dbg(DBG_CORE,"Proc%d do exit successfully\n", curproc -> p_pid);
    return NULL;
}

static void
killall_normal_run()
{
    dbg(DBG_CORE,"Test KILL_ALL_WITHOUT_DEADLOCK\n");
    int status[5];
    pid_t child[5];
    proc_t *process[5];
    kthread_t *thread[5];

    int i;
    for(i=0;i<5;i++)
    {
        process[i]=proc_create("test_process");
        thread[i]=kthread_create(process[i],test,0,NULL);
        sched_make_runnable(thread[i]);
    }

    proc_kill_all();
    i=0;
    while(!list_empty(&curproc->p_children))
    {
        child[i]=do_waitpid(-1,0,&status[i]);
        KASSERT(status[i]==0);
        dbg_print("process %d return.\n",(int)child[i]);
        i++;
    }
}

static void *
normal_test_run(int arg1, void *arg2)
{
    dbg_print("normal test function return.\n");
    return NULL;
}

static void *
test(int arg1, void *arg2)
{
    dbg_print("test busy wait function start.\n");
    while(1) {;}
    dbg_print("test busy wait function return.\n");
    return NULL;
}

static void
proc_kill_run()
{
    dbg(DBG_CORE,"Test PROC_KILL_TEST\n");
    dbg_print("The proces list at beginning\n");
    char buffer[1024];
    proc_list_info(NULL, buffer, 1024);
    dbg_print("%s", buffer);
    proc_t * process1=proc_create("test_process");
    kthread_t* thread1=kthread_create(process1,test,0,NULL);

    proc_t * process2=proc_create("test_process");
    kthread_t* thread2=kthread_create(process2,test,0,NULL);
    sched_make_runnable(thread1);
    sched_make_runnable(thread2);
    dbg_print("The proces list after create test process\n");
    proc_list_info(NULL, buffer, 1024);
    dbg_print("%s", buffer);

    proc_kill(process1,0);
    proc_kill(process2,0);
    dbg_print("The proces list after kill test process\n");
    proc_list_info(NULL, buffer, 1024);
    dbg_print("%s", buffer);
    /*proc_kill(curproc,0);*/
}


static void
deadlock_cancellable_run()
{
    dbg(DBG_CORE,"Test DEADLOCK_CANCELLABLE_TEST\n");
    proc_t *dead3proc,*dead4proc;
    kthread_t *dead3thr,*dead4thr;

    kmutex_init(&dead_mtx3);
    kmutex_init(&dead_mtx4);

    dead3proc=proc_create("dead3proc");
    dead3thr=kthread_create(dead3proc,deadlock3_run,0,NULL);
    dead4proc=proc_create("dead4proc");
    dead4thr=kthread_create(dead4proc,deadlock4_run,0,NULL);

    sched_make_runnable(dead3thr);
    sched_make_runnable(dead4thr);
}

static void *
deadlock3_run(int arg1, void *arg2)
{
    dbg_print("Proc%d trying to hold the mutex3\n", curproc -> p_pid);
    kmutex_lock_cancellable(&dead_mtx3);
    dbg_print("Proc%d holding the mutex3\n", curproc -> p_pid);
    sched_make_runnable(curthr);
    sched_switch();
    dbg_print("Proc%d trying to hold the mutex4\n", curproc -> p_pid);
    kmutex_lock_cancellable(&dead_mtx4);
    dbg_print("Proc%d holding the mutex4\n", curproc -> p_pid);
    kmutex_unlock(&dead_mtx4);
    kmutex_unlock(&dead_mtx3);
    return NULL;
}

static void *
deadlock4_run(int arg1, void *arg2)
{
    dbg_print("Proc%d trying to hold the mutex4\n", curproc -> p_pid);
    kmutex_lock_cancellable(&dead_mtx4);
    dbg_print("Proc%d holding the mutex4\n", curproc -> p_pid);
    sched_make_runnable(curthr);
    sched_switch();
    dbg_print("Proc%d trying to hold the mutex3\n", curproc -> p_pid);
    kmutex_lock_cancellable(&dead_mtx3);
    dbg_print("Proc%d holding the mutex3\n", curproc -> p_pid);
    kmutex_unlock(&dead_mtx3);
    kmutex_unlock(&dead_mtx4);
    return NULL;
}

static void
reader_writer_test() {
    dbg(DBG_CORE,"Test READER_WRITER_TEST\n");
    proc_t * preader, * pwriter;
    kthread_t * kreader, * kwriter;
    kmutex_init(&wr_mutex);
    sched_queue_init(&kt_readq);
    sched_queue_init(&kt_writeq);
    reading = 0;
    writing = 0;
    int i;
    /*create 3 writer*/
    for(i = 0; i < 3; i++) {
        char nwriter[8] = "writer";
        nwriter[6] = i + 1 + '0';
        nwriter[7] = '\0';
        pwriter = proc_create(nwriter);
        kwriter = kthread_create(pwriter,writer,0,NULL);
        sched_make_runnable(kwriter);
    }
    /*create 5 reader*/
    for(i = 0; i < 5; i++){
        char nreader[8] = "reader";
        nreader[6] = i + 1 + '0';
        nreader[7] = '\0';
        preader = proc_create(nreader);
        kreader = kthread_create(preader,reader,0,NULL);
        sched_make_runnable(kreader);
    }
}

static void *
reader(int arg1, void *arg2) {
    /*read the resource 3 times*/
    int i = 0;
    while(i < 3) {
        kmutex_lock(&wr_mutex);
        if(writing == 0) {
            reading++;
            dbg_print("%s read the resourcs\n", curproc -> p_comm);
            kmutex_unlock(&wr_mutex);
            reading--;
            if(reading == 0) {
                dbg_print("no reader now, wakeup the writer\n");
                sched_wakeup_on(&kt_writeq);
            }
            i++;
            sched_make_runnable(curthr);
            sched_switch();
        }
        else {
            dbg_print("%s waiting for the writer, block\n", curproc -> p_comm);
            kmutex_unlock(&wr_mutex);
            sched_sleep_on(&kt_readq);
        }
    }
    return NULL;
}

static void *
writer(int arg1, void *arg2) {
    /* write the resource 5 times */
    int i = 0;
    while(i < 5) {
        kmutex_lock(&wr_mutex);
        /*share_resource == 0 indicate no writer writing and no reader reading*/
        if(reading == 0 && writing == 0) {
            writing++;
            dbg_print("%s write the resourcs\n", curproc -> p_comm);
            kmutex_unlock(&wr_mutex);
            writing--;
            if(reading == 0 && writing == 0) {
                dbg_print("no reader and writer now, wakeup the writer or reader\n");
                sched_wakeup_on(&kt_writeq);
                sched_broadcast_on(&kt_readq);
            }
            i++;
            sched_make_runnable(curthr);
            sched_switch();
        }
        else {
            dbg_print("%s waiting for the writer or reader, block\n", curproc -> p_comm);
            kmutex_unlock(&wr_mutex);
            sched_sleep_on(&kt_writeq);
        }
    }
    return NULL;
}

static void
kshell_test() {
    dbg(DBG_CORE,"Test KSHELL_TEST\n");

    proc_t* pkshell = proc_create("kshell_test");
    kthread_t *tkshell = kthread_create(pkshell,kshell_run, 0, NULL);
    sched_make_runnable(tkshell);
}

static void *
kshell_run(int arg1, void *arg2) {
    /*dbg_print("Enter kshell_run\n");
    kshell_t *ksh=kshell_create(0);
    KASSERT(ksh && "kshell create failed.");
    int val=0;
    int err = 0;
    while((err=kshell_execute_next(ksh)) > 0);

    KASSERT(err == 0 && "kernel shell falsely exited.\n");
    kshell_destroy(ksh);
    dbg_print("Leave kshell_run\n");
    return NULL;*/

    /*kshell_t *new_shell;
    int i;
    while (1)
    {
        new_shell = kshell_create(0);
        i = kshell_execute_next(new_shell);
        if(i>0){dbg(DBG_TERM,"Error Executing the command\n");}
        kshell_destroy(new_shell);
        if(i==0){break;}
    }
    return NULL;*/
    int err = 0; 
    kshell_t *ksh = kshell_create(0); 
    KASSERT(ksh && "kshell create error");
    while ((err = kshell_execute_next(ksh)) > 0);
    KASSERT(err == 0 && "kshell exit error\n");
    kshell_destroy(ksh);

    return NULL;
}

static void *
deadlock1_run(int arg1, void *arg2)
{
    dbg_print("Proc%d trying to hold the mutex1\n", curproc -> p_pid);
    kmutex_lock(&dead_mtx1);
    dbg_print("Proc%d hold the mutex1\n", curproc -> p_pid);
    sched_make_runnable(curthr);
    sched_switch();
    dbg_print("Proc%d trying to hold the mutex2\n", curproc -> p_pid);
    kmutex_lock(&dead_mtx2);
    dbg_print("Proc%d hold the mutex2\n", curproc -> p_pid);
    kmutex_unlock(&dead_mtx2);
    dbg_print("Proc%d relese the mutex2\n", curproc -> p_pid);
    kmutex_unlock(&dead_mtx1);
    dbg_print("Proc%d relese the mutex1\n", curproc -> p_pid);
    return NULL;
}

static void *
deadlock2_run(int arg1, void *arg2)
{
    dbg_print("Proc%d trying to hold the mutex2\n", curproc -> p_pid);
    kmutex_lock(&dead_mtx2);
    dbg_print("Proc%d hold the mutex2\n", curproc -> p_pid);
    sched_make_runnable(curthr);
    sched_switch();
    dbg_print("Proc%d trying to hold the mutex1\n", curproc -> p_pid);
    kmutex_lock(&dead_mtx1);
    dbg_print("Proc%d hold the mutex1\n", curproc -> p_pid);
    kmutex_unlock(&dead_mtx1);
    dbg_print("Proc%d relese the mutex1\n", curproc -> p_pid);
    kmutex_unlock(&dead_mtx2);
    dbg_print("Proc%d relese the mutex2\n", curproc -> p_pid);
    return NULL;
}

static void
deadlock_run()
{
    dbg(DBG_CORE,"Test DEADLOCK_NORMAL_TEST\n");
    proc_t *dead1proc,*dead2proc;
    kthread_t *dead1thr,*dead2thr;

    kmutex_init(&dead_mtx1);
    kmutex_init(&dead_mtx2);

    dead1proc=proc_create("dead1proc");
    dead1thr=kthread_create(dead1proc,deadlock1_run,0,NULL);
    dead2proc=proc_create("dead2proc");
    dead2thr=kthread_create(dead2proc,deadlock2_run,0,NULL);

    sched_make_runnable(dead1thr);
    sched_make_runnable(dead2thr);

}

static void
producer_consmuser_test() {
    /* ----------------producer/consumer---------------------- */
    dbg(DBG_CORE,"Test PRODUCER_CONSMUER_TEST\n");
    proc_t * pproducer, * pconsumer;
    kthread_t * kproducer, * kconsumer;
    kmutex_init(&pc_mutex);
    pproducer = proc_create("producer");
    kproducer=kthread_create(pproducer,producer,0,NULL);
    pconsumer = proc_create("consumer");
    kconsumer=kthread_create(pconsumer,consumer,0,NULL);
    sched_queue_init(&kt_wproq);
    sched_queue_init(&kt_wconq);
    sched_make_runnable(kproducer);
    sched_make_runnable(kconsumer);
    /* ----------------producer/consumer---------------------- */
}

static void
deadlock_test() {
    /* ----------------deadlock---------------------- */
    dbg(DBG_CORE,"Test DEADLOCK_TEST\n");
    proc_t * proc1, * proc2;
    kthread_t * kthr1, * kthr2;
    kmutex_init(&mtx);
    proc1 = proc_create("proc1");
    kthr1=kthread_create(proc1,deadlock,0,NULL);
    proc2 = proc_create("proc2");
    kthr2=kthread_create(proc2,deadlock,0,NULL);
    sched_make_runnable(kthr1);
    sched_make_runnable(kthr2);
    /* ----------------deadlock---------------------- */
}

static void
normal_test() {
    /* create 10 procs for test */
    int i;
    for(i=0;i<10;i++)
    {
        proc_t *process = proc_create("test_process");
        kthread_t *thread = kthread_create(process,normal_test_run,0,NULL);
        sched_make_runnable(thread);
    }
}

static void *
producer(int arg1, void *arg2) {
    int i = 0;
    while(i < 10) {
        dbg_print("in producer proc\n");
        kmutex_lock(&pc_mutex);

        if(share_resource == 0) {
            share_resource = 1;
            i++;
            dbg_print("producer proc produce resourcs %d, wakeup consumer proc\n", i);
            kmutex_unlock(&pc_mutex);
            sched_wakeup_on(&kt_wconq);
        }
        else {
            dbg_print("producer proc wait for consumer proc\n");
            kmutex_unlock(&pc_mutex);
            sched_wakeup_on(&kt_wconq);
            sched_sleep_on(&kt_wproq);
        }
    }
    return NULL;
}

static void *
consumer(int arg1, void *arg2) {
    int i = 0;
    while(i < 10) {
        dbg_print("in consumer proc\n");
        kmutex_lock(&pc_mutex);

        if(share_resource == 1) {
            share_resource = 0;
            i++;
            dbg_print("consumer proc consume resourcs %d, wakeup producer proc\n", i);
            kmutex_unlock(&pc_mutex);
            sched_wakeup_on(&kt_wproq);
        }
        else {
            dbg_print("consumer proc wait for producer proc\n");
            kmutex_unlock(&pc_mutex);
            sched_wakeup_on(&kt_wproq);
            sched_sleep_on(&kt_wconq);
        }
    }
    return NULL;
}

static void *
deadlock(int arg1, void *arg2) {
    dbg_print("Proc%d trying to hold the mutex.\n", curproc -> p_pid);
    kmutex_lock(&mtx);
    dbg_print("Proc%d holding the mutex.\n", curproc -> p_pid);
    return NULL;
}

 /* Yu Sun Code Finish */

/**
 * Clears all interrupts and halts, meaning that we will never run
 * again.
 */
static void
hard_shutdown()
{
#ifdef __DRIVERS__
        vt_print_shutdown();
#endif
        __asm__ volatile("cli; hlt");
}


/*******************************************************************************************
 


 * Test for kernel assignment 2



 *******************************************************************************************/
 #include <errno.h>
 #include <fs/file.h>
 #include <fs/vnode.h>
 #include <fs/lseek.h>
 #include <mm/kmalloc.h>
 /* Some helpful strings */

#define LONGNAME "supercalifragilisticexpialidocious" /* Longer than NAME_LEN */

#define TESTSTR                                                                                 \
        "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do "                     \
        "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "         \
        "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "      \
        "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum "     \
        "dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, "     \
        "sunt in culpa qui officia deserunt mollit anim id est laborum."

#define SHORTSTR "Quidquid latine dictum, altum videtur"

/* declare char string of 'root durectiry' */
static char* root_dir;

/****************************** Test Helper *********************************/
/* create a file with the specified name */
static void
create_file(const char *filename)      
{                                                                
    dbg(DBG_VFS, "Enter creat_file().\n");
    int fd = do_open(filename, O_RDONLY|O_CREAT );
    if (fd >=0 && fd < NFILES)
    {             
            int ret;
            if ( (ret = do_close(fd)) == 0 )
                dbg(DBG_VFS, "Close the created file successfully.\n");
            else
                dbg(DBG_VFS, "Failed to close the created file, error: %d\n", ret);                                                
    }
    else
        dbg(DBG_VFS, "Failed to create file, error: %d\n", fd);                                                                         
}

static void
vfstest_start(void)
{
    int err;

    root_dir = "testDir";
    do {
            err = do_mkdir(root_dir);
    } while (err != 0);
    dbg(DBG_DISK, "Created test root directory: ./%s\n", root_dir);
}

static int
makedirs(const char *dir)
{
        char *d, *p;

        if (NULL == (d = kmalloc(strlen(dir) + 1))) 
                return -ENOMEM;
        strcpy(d, dir);

        p = d;
        int err;
        while (NULL != (p = strchr(p + 1, '/'))) 
        {
                *p = '\0';
                if ( (err = do_mkdir(d)) != 0 ) 
                        return err;
                *p = '/';
        }
        if ( (err = do_mkdir(d)) != 0 ) 
                return err;

        return 0;
}

static void
paths_equal(const char *p1, const char *p2)
{
    struct stat sp1, sp2;
    
    KASSERT( !makedirs(p1)                  );   
    
    KASSERT( !do_stat(p1, &sp1)             );
    KASSERT( !do_stat(p2, &sp2)             );
    KASSERT( sp1.st_ino == sp2.st_ino       );
    KASSERT( !do_rmdir(p1)                  );
    dbg(DBG_PROC, "paths_equals(\"%s\" (ino %d), \"%s\" (ino %d))", p1, sp1.st_ino, p2, sp2.st_ino);
}

static void
read_fd(int fd, ssize_t size, char *goal)
{
    void *buf = (void *) kmalloc(9999);
    KASSERT( (ssize_t)strlen(goal) == do_read(fd, buf, size) );
    KASSERT(    memcmp(buf, goal, strlen(goal)) == 0         );
}

static void
test_fpos(int fd, int exp)
{
    int g;
    g = do_lseek(fd, 0, SEEK_CUR);
    KASSERT(g == exp);
    dbg(DBG_VFS, "fd %d fpos at %d, expected %d", fd, g, exp);
}




static void
display_node_create(const char *filename)
{
    struct stat s;  
    do_stat(filename, &s);
    dbg(DBG_USER, "\"%s\" has been created, ino: %d\n", filename, s.st_ino);
}

static void
display_node_remove(const char *filename)
{
    struct stat s;  
    do_stat(filename, &s);
    dbg(DBG_USER, "\"%s\" is to be removed, ino: %d\n", filename, s.st_ino);
}

static int
removeall(const char *dir)
{
        int ret, fd = -1;
        dirent_t dirent;
        struct stat status;
        display_node_create(dir);
        if ( (ret = do_chdir(dir)) != 0)            
            return ret;

        ret = 1;
        while (ret != 0) 
        {
                if ( (ret = do_stat(dir, &status)) != 0)
                        goto error;

                if (S_ISDIR(status.st_mode)) 
                {
                        if ( (ret = removeall(dir)) != 0)
                                goto error;
                } 
                else 
                {
                        if ( (ret = do_unlink(dir)) != 0)
                                goto error;
                }
        }

        if ( (ret = do_chdir("..")) != 0 )
                return ret;
        display_node_remove(dir);
        if ( (ret = do_rmdir(dir)) != 0)
                return ret;

        return 0;

error:
        return ret;
}

static int
getdents(int fd, struct dirent *dirp, unsigned int count)
{
        size_t numbytesread = 0;
        int ret = 0;
        dirent_t tempdirent;

        if (count < sizeof(dirent_t)) 
        {
                return -EINVAL;;
        }

        while (numbytesread < count) 
        {
                if ((ret = do_getdent(fd, &tempdirent)) < 0) 
                {
                        return ret;
                }
                if (ret == 0) {
                        return numbytesread;
                }
                memcpy(dirp, &tempdirent, sizeof(dirent_t));

                KASSERT(ret == sizeof(dirent_t));

                dirp++;
                numbytesread += ret;
        }
        return numbytesread;
}

/****************************** Syscall Function Test *********************************/
/* vfs test of do_stat */
static void
vfstest_stat(void)
{
        int fd;
        struct stat s;
        dbg(DBG_USER, "//---------------- VFSTEST_STAT Begins---------------------------------------------//\n");
        KASSERT( !do_mkdir("stat")    ); 
        display_node_create ("stat");  
        KASSERT( !do_chdir("stat")    );
        KASSERT( !do_stat(".", &s)    );
        KASSERT( S_ISDIR(s.st_mode)   );

        create_file("file");
        display_node_create ("file"); 

        KASSERT( !do_stat("file", &s) );
        KASSERT( S_ISREG(s.st_mode)   );
        dbg(DBG_USER, "file mode is correct!\n");

        /* file size is correct */
        fd = do_open("file", O_RDWR);
        file_t *file = fget(fd);
        fput(file);
        do_write(fd, "foobar", 6);
        KASSERT( !do_stat("file", &s) );
        KASSERT( s.st_size == 6       );
        do_close(fd);
        dbg(DBG_USER, "file size is correct!\n");

        /* no entry test */
        KASSERT( do_stat("noent", &s) == -ENOENT );
        dbg(DBG_USER, "No entry test. Works!\n");

        do_chdir("..");
}

/* vfs test of do_mkdir */
static void
vfstest_mkdir(void)
{
        dbg(DBG_USER, "//---------------- VFSTEST_MKDIR Begins--------------------------------------//\n");        
        KASSERT( !do_mkdir("mkdir") );
        display_node_create("mkdir");
        KASSERT( !do_chdir("mkdir") );

        /* mkdir an existing file or directory */
        create_file("file");
        display_node_create("file");
        KASSERT( do_mkdir("file")       == -EEXIST      );
        KASSERT( !do_mkdir("dir")           );
        display_node_create("dir");
        KASSERT( do_mkdir("dir")        == -EEXIST      );
        dbg(DBG_USER, "mkdir an existing file or directory. Works!\n");

        /* mkdir an invalid path */
        KASSERT( do_mkdir(LONGNAME)     == -ENAMETOOLONG);
        KASSERT( do_mkdir("file/dir")   == -ENOTDIR     );
        KASSERT( do_mkdir("noent/dir")  == -ENOENT      );
        KASSERT( do_rmdir("file/dir")   == -ENOTDIR     );
        KASSERT( do_rmdir("noent/dir")  == -ENOENT      );
        KASSERT( do_rmdir("noent")      == -ENOENT      );
        KASSERT( do_rmdir(".")          == -EINVAL      );
        KASSERT( do_rmdir("..")         == -ENOTEMPTY   );
        KASSERT( do_rmdir("dir/.")      == -EINVAL      );
        dbg(DBG_USER, "mkdir an invalid path. Works!\n");

        /* unlink and rmdir the inappropriate types */
        KASSERT( do_rmdir("file")       == -ENOTDIR     );
        KASSERT( do_unlink("dir")       == -EISDIR      );
        dbg(DBG_USER, "Unlink and rmdir the inappropriate types. Works!\n");

        /* remove non-empty directory */
        create_file("dir/file");
        display_node_create("dir/file");
        KASSERT( do_rmdir("dir")        == -ENOTEMPTY   );
        dbg(DBG_USER, "Remove non-empty directory. Works!\n");
        
        /* remove empty directory */
        display_node_remove("dir/file");
        KASSERT( !do_unlink("dir/file"));
        KASSERT( !do_rmdir("dir"));
        dbg(DBG_USER, "Remove empty directory. Works!\n");

        KASSERT( !do_chdir(".."));
}

static void
vfstest_chdir(void)
{
#define CHDIR_TEST_DIR "chdir"
        dbg(DBG_USER, "//---------------- VFSTEST_CHDIR Begins -------------------------------------//\n");
        struct stat ssrc, sdest, sparent, sdir;
        struct stat rsrc, rdir;

        /* chdir back and forth to CHDIR_TEST_DIR */
        KASSERT( !do_mkdir(CHDIR_TEST_DIR)      );
        display_node_create (CHDIR_TEST_DIR); 
        KASSERT( !do_stat(".", &ssrc)           );
        KASSERT( !do_stat(CHDIR_TEST_DIR, &sdir));

        KASSERT( ssrc.st_ino != sdir.st_ino     );

        KASSERT( !do_chdir(CHDIR_TEST_DIR)      );
        KASSERT( !do_stat(".", &sdest)          );
        KASSERT( !do_stat("..", &sparent)       );

        KASSERT( sdest.st_ino == sdir.st_ino    );
        KASSERT( ssrc.st_ino  == sparent.st_ino );
        KASSERT( ssrc.st_ino  != sdest.st_ino   );

        KASSERT( !do_chdir("..")                );
        KASSERT( !do_stat(".", &rsrc)           );
        KASSERT( !do_stat(CHDIR_TEST_DIR, &rdir));

        KASSERT( rsrc.st_ino == ssrc.st_ino     );
        KASSERT( rdir.st_ino == sdir.st_ino     );
        dbg(DBG_USER, "Chdir back and forth to CHDIR_TEST_DIR. Works!\n");

        /* can't chdir into non-directory */
        KASSERT( !do_chdir(CHDIR_TEST_DIR)      );
        create_file("file");

        int ret = do_chdir("file");
        KASSERT( ret == -ENOTDIR   );
        KASSERT( do_chdir("noent")== -ENOENT    );
        dbg(DBG_USER, "Can't chdir into non-directory. Works!\n");

        KASSERT( !do_chdir("..")                );
}


static void
vfstest_paths(void)
{
#define PATHS_TEST_DIR "paths"
    dbg(DBG_USER, "//---------------- VFSTEST_PATHS Begins---------------------------------------------//\n");
        struct stat s;

        KASSERT( !do_mkdir(PATHS_TEST_DIR)      );
        display_node_create(PATHS_TEST_DIR);
        KASSERT( !do_chdir(PATHS_TEST_DIR)      );

        KASSERT( do_stat("", &s) == -EINVAL     );

        paths_equal("a/b/c", "a/b/c");
        paths_equal("q/w/e", "q/w/e");

        paths_equal("1/2/3", "1/2/.//3");
        paths_equal("4/5/6", "4/./5/./../5/6");
        paths_equal("7/8/9", "7/8//9");
        paths_equal("10/11/12", "10//11//12");
        paths_equal("13/14/15", "13//14//15/");
        paths_equal("16/17/18", "16///17///18///");

        paths_equal("-", "-");
        paths_equal(" ", " ");
        paths_equal("\\", "\\");
        paths_equal("0", "0");

        struct stat st;

        KASSERT( do_stat("asdf", &st)       == -ENOENT  );
        KASSERT( do_stat("1/asdf", &st)     == -ENOENT  );
        KASSERT( do_stat("1/../asdf", &st)  == -ENOENT  );
        KASSERT( do_stat("1/2/asdf", &st)   == -ENOENT  );

        create_file("1/file");
        KASSERT( do_open("1/file/other", O_RDONLY) == -ENOTDIR);
        KASSERT( do_open("1/file/other", O_RDONLY | O_CREAT) == -ENOTDIR);

        KASSERT( !do_chdir(".."));
}

static void
vfstest_fd(void)
{
#define FD_BUFSIZE 5
#define BAD_FD 20
#define HUGE_FD 9999
        dbg(DBG_USER, "//---------------- VFSTEST_FD Begins---------------------------------------------//\n");
        int fd1, fd2;
        char buf[FD_BUFSIZE];
        struct dirent d;

        KASSERT( !do_mkdir("fd")    );
        display_node_create("fd");
        KASSERT( !do_chdir("fd")    );

        /* read/write/close/getdents/dup nonexistent file descriptors */
        KASSERT( do_read(BAD_FD, buf, FD_BUFSIZE)   == -EBADF   );
        KASSERT( do_read(HUGE_FD, buf, FD_BUFSIZE)  == -EBADF   );
        KASSERT( do_read(-1, buf, FD_BUFSIZE)       == -EBADF   );

        KASSERT( do_write(BAD_FD, buf, FD_BUFSIZE)  == -EBADF   );
        KASSERT( do_write(HUGE_FD, buf, FD_BUFSIZE) == -EBADF   );
        KASSERT( do_write(-1, buf, FD_BUFSIZE)      == -EBADF   );

        KASSERT( do_close(BAD_FD)                   == -EBADF   );
        KASSERT( do_close(HUGE_FD)                  == -EBADF   );
        KASSERT( do_close(-1)                       == -EBADF   );

        KASSERT( do_lseek(BAD_FD, 0, SEEK_SET)      == -EBADF   );
        KASSERT( do_lseek(HUGE_FD, 0, SEEK_SET)     == -EBADF   );
        KASSERT( do_lseek(-1, 0, SEEK_SET)          == -EBADF   );

        KASSERT( do_getdent(BAD_FD, &d)             == -EBADF   );
        KASSERT( do_getdent(HUGE_FD, &d)            == -EBADF   );
        KASSERT( do_getdent(-1, &d)                 == -EBADF   );

        KASSERT( do_dup(BAD_FD)                     == -EBADF   );
        KASSERT( do_dup(HUGE_FD)                    == -EBADF   );
        KASSERT( do_dup(-1)                         == -EBADF   );

        KASSERT( do_dup2(BAD_FD, 10)                == -EBADF   );
        KASSERT( do_dup2(HUGE_FD, 10)               == -EBADF   );
        KASSERT( do_dup2(-1, 10)                    == -EBADF   );
        dbg(DBG_USER, "Read/write/close/getdents/dup nonexistent file descriptors, works!\n");

        /* dup2 has some extra cases since it takes a second fd */
        KASSERT( do_dup2(0, HUGE_FD)                == -EBADF   );
        KASSERT( do_dup2(0, -1)                     == -EBADF   );
        dbg(DBG_USER, "Dup2 has some extra cases since it takes a second fd, works!\n");

        /* if the fds are equal, but the first is invalid or out of the
         * allowed range */
        KASSERT( do_dup2(BAD_FD, BAD_FD)            == -EBADF   );
        KASSERT( do_dup2(HUGE_FD, HUGE_FD)          == -EBADF   );
        KASSERT( do_dup2(-1, -1)                    == -EBADF   );

        /* dup works properly in normal usage */
        create_file("file01");
        fd1 = do_open("file01", O_RDWR);
        fd2 = do_dup(fd1);
        KASSERT( fd1 < fd2);
        do_write(fd2, "hello", 5);
        test_fpos(fd1, 5); test_fpos(fd2, 5);
        do_lseek(fd2, 0, SEEK_SET);
        test_fpos(fd1, 0); test_fpos(fd2, 0);
        read_fd(fd1, 5, "hello"); 
        test_fpos(fd1, 5); test_fpos(fd2, 5);
        KASSERT( !do_close(fd2) );
        dbg(DBG_USER, "Dup works properly in normal usage!\n");

        /* dup2 works properly in normal usage */
        fd2 = do_dup2(fd1, 10);
        KASSERT( 10 == fd2  );
        dbg(DBG_USER, "Dup2 works properly in normal usage!\n");

        test_fpos(fd1, 5); test_fpos(fd2, 5);
        do_lseek(fd2, 0, SEEK_SET);
        test_fpos(fd1, 0); test_fpos(fd2, 0);
        KASSERT( !do_close(fd2) );

        /* dup2-ing a file to itself works */
        fd2 = do_dup2(fd1, fd1);
        KASSERT( fd1 == fd2 );
        KASSERT( !do_close(fd2) );
        dbg(DBG_USER, "Dup2-ing a file to itself. Works!\n");

        /* dup2 closes previous file */
        int fd3;
        create_file("file02");
        fd3 = do_open("file02", O_RDWR);
        fd2 = do_dup2(fd1, fd3);
        KASSERT( fd2 == fd3 );
        test_fpos(fd1, 0); test_fpos(fd2, 0);
        do_lseek(fd2, 5, SEEK_SET);
        test_fpos(fd1, 5); test_fpos(fd2, 5);
        dbg(DBG_USER, "Dup2 closing previous file. Works!\n");
        
        KASSERT( !do_chdir("..")    );
}



static void
vfstest_open(void)
{
#define OPEN_BUFSIZE 5

        char buf[OPEN_BUFSIZE];
        int fd, fd2;
        struct stat s;
        dbg(DBG_USER, "//---------------- VFSTEST_OPEN Begins---------------------------------------------//\n");
        KASSERT( !do_mkdir("open")      );
        display_node_create("open");
        KASSERT( !do_chdir("open")      );

        /* No invalid combinations of O_RDONLY, O_WRONLY, and O_RDWR.  Since
         * O_RDONLY is stupidly defined as 0, the only invalid possible
         * combination is O_WRONLY|O_RDWR. */
        KASSERT( do_open("file01", O_WRONLY | O_RDWR | O_CREAT)            == -EINVAL   );
        KASSERT( do_open("file01", O_RDONLY | O_RDWR | O_WRONLY | O_CREAT) == -EINVAL   );
        dbg(DBG_USER, "FILE: \"file01\" : INVALID flag. Works!\n");

        /* Cannot open nonexistent file without O_CREAT */
        KASSERT( do_open("file02", O_WRONLY) == -ENOENT);
        fd = do_open("file02", O_RDONLY | O_CREAT);
        display_node_create("file02");
        KASSERT( !do_close(fd)                      );
        display_node_remove("file02");
        KASSERT( !do_unlink("file02")               );
        KASSERT( do_stat("file02", &s)  == -ENOENT  );
        dbg(DBG_USER, "FILE: \"file02\" : Cannot open nonexistent file without O_CREAT. Works!\n");

        /* Cannot create invalid files */
        create_file("tmpfile");
        display_node_create("tmpfile");
        KASSERT( do_open("tmpfile/test", O_RDONLY | O_CREAT) == -ENOTDIR    );
        KASSERT( do_open("noent/test", O_RDONLY | O_CREAT)   == -ENOENT     );
        KASSERT( do_open(LONGNAME, O_RDONLY | O_CREAT)       == -ENAMETOOLONG);
        dbg(DBG_USER, "FILE: \"tmpfile\" : Cannot create invalid files. Works!\n");

        /* Cannot write to readonly file */
        fd = do_open("file03", O_RDONLY | O_CREAT);
        display_node_create("file03");
        KASSERT( do_write(fd, "hello", 5) == -EBADF );
        KASSERT( !do_close(fd)                      );
        dbg(DBG_USER, "FILE: \"file03\" : Cannot write to readonly file. Works!\n");

        /* Cannot read from writeonly file.  Note that we do not unlink() it
         * from above, so we do not need O_CREAT set. */
        fd = do_open("file03", O_WRONLY);
        KASSERT( do_read(fd, buf, OPEN_BUFSIZE) == -EBADF);
        KASSERT( !do_close(fd)                      );
        display_node_remove("file03");
        KASSERT( !do_unlink("file03")               );
        KASSERT( do_stat("file03", &s)   == -ENOENT );
        dbg(DBG_USER, "FILE: \"file03\" : Cannot read from writeonly file. Works!\n");

        /* Lowest file descriptor is always selected. */
        fd  = do_open("file04", O_RDONLY | O_CREAT);
        display_node_create("file04");
        fd2 = do_open("file04", O_RDONLY);
        KASSERT( fd2 > fd                       );
        KASSERT( !do_close(fd)                  );
        KASSERT( !do_close(fd2)                 );
        fd2 = do_open("file04", O_WRONLY);
        KASSERT( fd2 == fd );
        KASSERT( !do_close(fd2)                 );
        display_node_remove("file04");
        KASSERT( !do_unlink("file04")           );
        KASSERT( do_stat("file04", &s)== -ENOENT);
        dbg(DBG_USER, "FILE: \"file04\" : Lowest file descriptor is always selected. Works!\n");

        /* Cannot open a directory for writing */
        KASSERT( !do_mkdir("file05") );
        display_node_create("file05");                      
        KASSERT( do_open("file05", O_WRONLY) == -EISDIR);
        KASSERT( do_open("file05", O_RDWR)   == -EISDIR);
        display_node_remove("file05");
        KASSERT( !do_rmdir("file05")                );
        dbg(DBG_USER, "DIR: \"file05\" : Cannot open a directory for writing. Works!\n");

        /* Cannot unlink a directory */
        KASSERT( !do_mkdir("file06") );
        display_node_create("file06");                  
        KASSERT( do_unlink("file06")      == -EISDIR);
        display_node_remove("file06");
        KASSERT( !do_rmdir("file06")                );
        dbg(DBG_USER, "DIR: \"file06\" : Cannot unlink a directory. Works!\n");

        /* Cannot unlink a non-existent file */
        KASSERT( do_unlink("file07") == -ENOENT     );
        dbg(DBG_USER, "FILE: \"file07\" : Cannot unlink a non-existent file. Works!\n");

        KASSERT( !do_chdir("..")                    );
}

static void
vfstest_read(void)
{
#define READ_BUFSIZE 256
        int fd, ret;
        char buf[READ_BUFSIZE];
        struct stat s;
        dbg(DBG_USER, "//---------------- VFSTEST_READ Begins------------------------------------------//\n");
        KASSERT( !do_mkdir("read")                          );
        display_node_create("read");
        KASSERT( !do_chdir("read")                          );

        fd  = do_open("file01", O_RDWR | O_CREAT);
        ret = do_write(fd, "hello", 5);
        dbg_print("do_write(), ret=%d\n", ret);
        KASSERT(    5 == ret                                );
        ret = do_lseek(fd, 0, SEEK_SET);
        KASSERT(    0 == ret                                );
        read_fd(fd, READ_BUFSIZE, "hello");
        KASSERT( !do_close(fd)                              );
        dbg(DBG_USER, "FILE: \"file01\" : Can read and write to a file. Works!\n");

       
        KASSERT( !do_mkdir("dir01")                         );
        fd = do_open("dir01", O_RDONLY);
        KASSERT( do_read(fd, buf, READ_BUFSIZE)  == -EISDIR );
        KASSERT( !do_close(fd)                              );
        dbg(DBG_USER, "DIR: \"dir01\" : Cannot read from a directory. Works!\n");

       
        fd = do_open("file02", O_RDWR | O_CREAT);
        KASSERT( do_write(fd, "hello", 5) == 5              );

        KASSERT( do_lseek(fd, 0, SEEK_CUR)  == 5        );
        read_fd(fd, 10, "");
        KASSERT( do_lseek(fd, -1, SEEK_CUR) == 4        );
        read_fd(fd, 10, "o");
        KASSERT( do_lseek(fd, 2, SEEK_CUR)  == 7        );
        read_fd(fd, 10, "");
        KASSERT( do_lseek(fd, -8, SEEK_CUR) == -EINVAL  );

        KASSERT( do_lseek(fd, 0, SEEK_SET)  == 0        );
        read_fd(fd, 10, "hello");
        KASSERT( do_lseek(fd, 3, SEEK_SET)  == 3        );
        read_fd(fd, 10, "lo");
        KASSERT( do_lseek(fd, 7, SEEK_SET)  == 7        );
        read_fd(fd, 10, "");
        KASSERT( do_lseek(fd, -1, SEEK_SET) == -EINVAL  );

        KASSERT( do_lseek(fd, 0, SEEK_END)  == 5        );
        read_fd(fd, 10, "");
        KASSERT( do_lseek(fd, -2, SEEK_END) == 3        );
        read_fd(fd, 10, "lo");
        KASSERT( do_lseek(fd, 3, SEEK_END)  == 8        );
        read_fd(fd, 10, "");
        KASSERT( do_lseek(fd, -8, SEEK_END) == -EINVAL  );

        KASSERT( do_lseek(fd, 0, SEEK_SET + SEEK_CUR + SEEK_END) == -EINVAL );
        KASSERT( !do_close(fd)                          );
        dbg(DBG_USER, "FILE: \"file02\": Can seek to beginning, middle, and end of file. Works!\n");

        
        create_file("file03");
        fd = do_open("file03", O_RDWR);
        test_fpos(fd, 0);
        do_write(fd, "hello", 5);
        test_fpos(fd, 5);
        KASSERT( !do_close(fd)  );

        fd = do_open("file03", O_RDWR | O_APPEND);
        test_fpos(fd, 0);
        do_write(fd, "hello", 5);
        test_fpos(fd, 10);

        do_lseek(fd, 0, SEEK_SET);
        test_fpos(fd, 0);
        read_fd(fd, 10, "hellohello");
        do_lseek(fd, 5, SEEK_SET);
        test_fpos(fd, 5);
        do_write(fd, "again", 5);
        test_fpos(fd, 15);
        do_lseek(fd, 0, SEEK_SET);
        test_fpos(fd, 0);
        read_fd(fd, 15, "hellohelloagain");
        KASSERT( !do_close(fd)  );
        dbg(DBG_USER, "FILE: \"file03\":  O_APPEND works properly\n");
        
        create_file("file04");
        fd = do_open("file04", O_RDWR);
        do_write(fd, "hello", 5);
        test_fpos(fd, 5);
        KASSERT( do_lseek(fd, 10, SEEK_SET) == 10       );
        do_write(fd, "again", 5);
        KASSERT( !do_stat("file04", &s)                 );
        KASSERT( s.st_size == 15                        );
        KASSERT( do_lseek(fd, 0, SEEK_SET)  == 0        );
        KASSERT( 15 == do_read(fd, buf, READ_BUFSIZE)   );
        KASSERT( 0  == memcmp(buf, "hello\0\0\0\0\0again", 15));
        KASSERT( !do_close(fd)  );
        dbg(DBG_USER, "FILE: \"file04\":  seek and write beyond end of file. Works!\n");

        KASSERT( !do_chdir("..")                        );
}

/*
 * Terminates the testing environment
 */
static void
vfstest_term(void)
{
        dbg(DBG_USER, "//----------------  VFSTEST_TERM Begins ------------------------------//\n");   
        int ret;        
        if ( (ret = removeall(root_dir)) != 0 ) 
                dbg(DBG_USER, "ERROR: could not remove testing root %s, #error: %d\n", root_dir, ret);
        else
                dbg(DBG_USER, "Removed test root directory: ./%s\n", root_dir);
}

static void
vfstest_getdents(void)
{
        int fd, ret;
        dirent_t dirents[4];
        dbg(DBG_USER, "//----------------  VFSTEST_GETDENTS Begins --------------------------//\n");
        KASSERT( !do_mkdir("getdents")               );
        display_node_create("getdents");
        KASSERT( !do_chdir("getdents")               );

        /* getdents works */
        KASSERT( !do_mkdir("dir01")                  );
        display_node_create("dir01");
        KASSERT( !do_mkdir("dir01/1")                );
        display_node_create("dir01/1");
        create_file("dir01/2");
        display_node_create("dir01/2");

        fd = do_open("dir01", O_RDONLY);
        ret = getdents(fd, dirents, 4 * sizeof(dirent_t));
        KASSERT( 4 * sizeof(dirent_t) == ret );

        ret = getdents(fd, dirents, sizeof(dirent_t));
        KASSERT( 0 == ret );

        do_lseek(fd, 0, SEEK_SET);
        test_fpos(fd, 0);
        ret = getdents(fd, dirents, 2 * sizeof(dirent_t));
        KASSERT( 2 * sizeof(dirent_t) == ret );
        ret = getdents(fd, dirents, 2 * sizeof(dirent_t));
        KASSERT( 2 * sizeof(dirent_t) == ret );
        ret = getdents(fd, dirents, sizeof(dirent_t));
        KASSERT( 0 == ret );
        KASSERT( !do_close(fd)                      );
        dbg(DBG_USER, "Getdents works!\n");

        /* Cannot call getdents on regular file */
        create_file("file01");
        display_node_create("file01");
        fd = do_open("file01", O_RDONLY);
        KASSERT( getdents(fd, dirents, 4 * sizeof(dirent_t)) == -ENOTDIR );
        KASSERT( !do_close(fd)                      );
        dbg(DBG_USER, "Cannot call getdents on regular file. Works!\n");

        KASSERT( !do_chdir(".."));
}

/* These operations should run for a long time and halt when the file
 * descriptor overflows. */
static void
vfstest_infinite(void)
{
        dbg(DBG_USER, "//----------------  VFSTEST_INFINITE Begins --------------------------//\n");
        int res, fd;
        char buf[PAGE_SIZE];

        res = 1;
        fd = do_open("/dev/null", O_WRONLY);
        file_t *f;  
        f = fget(fd);
        fput(f);
        while (0 < res) 
        {
                res = do_write(fd, buf, sizeof(buf));
                dbg(DBG_PRINT, "File cursor position: %d\n", f->f_pos);
        }
        KASSERT( !do_close(fd) );

        res = 1;
        fd = do_open("/dev/zero", O_RDONLY);
        while (0 < res) 
        {
                res = do_read(fd, buf, sizeof(buf));
        }
        KASSERT( !do_close(fd) );
}



/****************************** Main *********************************/
static void *
vfs_test() 
{   
    /* begin vfs test*/
    vfstest_start();
    do_chdir(root_dir);
    vfstest_stat();
    vfstest_mkdir();
    vfstest_chdir();
    vfstest_paths();
    vfstest_fd();
    vfstest_open();
    vfstest_read();
    vfstest_getdents();

    /*vfstest_infinite();*/
    return 0;
}


static void
vfs_test_run() {
    dbg(DBG_CORE,"Test VFS_TEST\n");
    pid_t child=0;
    int status=0;
    proc_t * process=proc_create("vfs_process");
    kthread_t* thread=kthread_create(process,vfs_test,0,NULL);
    sched_make_runnable(thread);

    child=do_waitpid(-1,0,&status);
    KASSERT(status==0);
    dbg_print("process %d return.\n",(int)child);
}
