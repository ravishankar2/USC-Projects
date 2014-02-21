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

static int CURRENT_TEST = PROC_EXIT_TEST;

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

    kshell_t *new_shell;
    int i;
    while (1)
    {
        new_shell = kshell_create(0);
        i = kshell_execute_next(new_shell);
        if(i>0){dbg(DBG_TERM,"Error Executing the command\n");}
        kshell_destroy(new_shell);
        if(i==0){break;}
    }
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
