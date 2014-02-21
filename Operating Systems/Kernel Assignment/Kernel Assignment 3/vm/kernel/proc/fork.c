#include "types.h"
#include "globals.h"
#include "errno.h"

#include "util/debug.h"
#include "util/string.h"

#include "proc/proc.h"
#include "proc/kthread.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/pframe.h"
#include "mm/mmobj.h"
#include "mm/pagetable.h"
#include "mm/tlb.h"

#include "fs/file.h"
#include "fs/vnode.h"

#include "vm/shadow.h"
#include "vm/vmmap.h"

#include "api/exec.h"

#include "main/interrupt.h"

/* Pushes the appropriate things onto the kernel stack of a newly forked thread
 * so that it can begin execution in userland_entry.
 * regs: registers the new thread should have on execution
 * kstack: location of the new thread's kernel stack
 * Returns the new stack pointer on success. */
static uint32_t
fork_setup_stack(const regs_t *regs, void *kstack)
{
        /* Pointer argument and dummy return address, and userland dummy return
         * address */
        uint32_t esp = ((uint32_t) kstack) + DEFAULT_STACK_SIZE - (sizeof(regs_t) + 12);
        *(void **)(esp + 4) = (void *)(esp + 8); /* Set the argument to point to location of struct on stack */
        memcpy((void *)(esp + 8), regs, sizeof(regs_t)); /* Copy over struct */
        return esp;
}


/*
 * The implementation of fork(2). Once this works,
 * you're practically home free. This is what the
 * entirety of Weenix has been leading up to.
 * Go forth and conquer.
 */
int
do_fork(struct regs *regs)
{
        dbg(DBG_USER,"GRADING: KASSERT (regs != NULL) is going getting invoked right now ! \n");
		KASSERT (regs != NULL);
		dbg(DBG_USER,"GRADING: I've made it ! May I have 2 points please ! \n");

		dbg(DBG_USER,"GRADING: KASSERT (curproc != NULL) is going getting invoked right now ! \n");
		KASSERT (curproc != NULL);
		dbg(DBG_USER,"GRADING: I've made it ! May I have 2 points please ! \n");

		dbg(DBG_USER,"GRADING: KASSERT (curproc->p_state == PROC_RUNNING) is going getting invoked right now ! \n");
		KASSERT (curproc->p_state == PROC_RUNNING);
		dbg(DBG_USER,"GRADING: I've made it ! May I have 2 points please ! \n");

		proc_t *process = proc_create("process");
		process->p_vmmap = vmmap_clone(curproc->p_vmmap);
		vmarea_t *parent_vmarea;
		list_iterate_begin(&(curproc->p_vmmap->vmm_list), parent_vmarea, vmarea_t, vma_plink)
		{
			if (parent_vmarea->vma_flags == MAP_PRIVATE)
			{
				mmobj_t *obj = shadow_create();
				obj = parent_vmarea->vma_obj;
				parent_vmarea->vma_obj = obj;	
			}
		}
		list_iterate_end();
		vmarea_t *child_vmarea;
		list_iterate_begin(&(process->p_vmmap->vmm_list), child_vmarea, vmarea_t, vma_plink)
		{
			if (child_vmarea->vma_flags == MAP_PRIVATE)
			{
				mmobj_t *obj = shadow_create();
				obj = parent_vmarea->vma_obj;
				child_vmarea->vma_obj= obj;
			}
		}
		list_iterate_end();
		pt_unmap_range(curproc->p_pagedir, 0, 0);
		tlb_flush_all();

		kthread_t *child_thread = kthread_create(process, NULL, 0, NULL);
		child_thread = kthread_clone(curthr);

		dbg(DBG_USER,"GRADING: KASSERT(newproc->p_state == PROC_RUNNING), is going getting invoked right now ! \n");
		KASSERT(process->p_state == PROC_RUNNING);
		dbg(DBG_USER,"GRADING: I've made it ! May I have 2 points please ! \n");

		dbg(DBG_USER,"GRADING: KASSERT(newproc->p_pagedir != NULL), is going getting invoked right now ! \n");
		KASSERT(process->p_pagedir !=NULL);
		dbg(DBG_USER,"GRADING: I've made it ! May I have 2 points please ! \n");

		dbg(DBG_USER,"GRADING: KASSERT(newthr->kt_kstack != NULL), is going getting invoked right now ! \n");
		KASSERT(child_thread->kt_kstack != NULL);
		dbg(DBG_USER,"GRADING: I've made it ! May I have 2 points please ! \n");

	    child_thread->kt_proc = process; 
		(child_thread->kt_ctx).c_eip = (uint32_t)userland_entry;
		(child_thread->kt_ctx).c_esp = fork_setup_stack(regs, child_thread->kt_kstack);

		process->p_cwd = curproc->p_cwd;
		return 0;
}
