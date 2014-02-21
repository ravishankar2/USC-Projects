#include "globals.h"
#include "errno.h"
#include "types.h"

#include "mm/mm.h"
#include "mm/tlb.h"
#include "mm/mman.h"
#include "mm/page.h"

#include "proc/proc.h"

#include "util/string.h"
#include "util/debug.h"

#include "fs/vnode.h"
#include "fs/vfs.h"
#include "fs/file.h"

#include "vm/vmmap.h"
#include "vm/mmap.h"

/*
 * This function implements the mmap(2) syscall, but only
 * supports the MAP_SHARED, MAP_PRIVATE, MAP_FIXED, and
 * MAP_ANON flags.
 *
 * Add a mapping to the current process's address space.
 * You need to do some error checking; see the ERRORS section
 * of the manpage for the problems you should anticipate.
 * After error checking most of the work of this function is
 * done by vmmap_map(), but remember to clear the TLB.
 */
int
do_mmap(void *addr, size_t len, int prot, int flags,
        int fd, off_t off, void **ret)
{
		/* Invalid flags */
        if(flags != MAP_SHARED && flags != MAP_PRIVATE && flags != MAP_FIXED && flags != MAP_ANON)
        {
				dbg(DBG_ERROR | DBG_VM, "ERROR: do_mmap: Invalid flags\n");
				return EINVAL;
		}

		/* Invalid file descriptor */
        if(fd < 0 || fd >= NFILES || (curproc->p_files[fd]==NULL))
        {
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_mmap: Invalid file descriptor\n");
            return -EBADF;       
        }
        
        /* Invalid prot, PROT_EXEC should be pass */
        if(prot != PROT_EXEC)
        {
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_mmap: Invalid prot\n");
            return -EPERM;
		}

		/* Invalid len, len should be not be NULL */
		if (sizeof(len) == NULL)
		{
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_mmap: Invalid prot\n");
            return -EINVAL; 

		}

		/* Invalid off, off should be not be NULL */
		if (sizeof(off) == NULL)
		{
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_mmap: Invalid prot\n");
            return -EINVAL; 

		}

		/* Ivalid addr */
		if(PAGE_ALIGNED(addr)==0)
		{
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_mmap: Invalid addr\n");
            return -EINVAL; 
		}

		/* The system limit on the number of open files */
		if(fd == -EMFILE)
		{
			dbg(DBG_ERROR | DBG_VM,"ERROR: do_map: The current process pid= %d exceeds the maximum permissible number of files.\n",curproc->p_pid);
			return -EMFILE;
		}

		/* Insufficient Memory */
		file_t *fresh_file = fget(-1);
		if(fresh_file == NULL)
		{
			dbg(DBG_ERROR | DBG_VM,"ERROR: do_map: No kernel memory availble\n");
			fput(fresh_file);
			return -ENOMEM;
		}

		/* Refer to the man page of mmap2 */
		if(flags == MAP_PRIVATE && curproc->p_files[fd]->f_mode != FMODE_READ)
		{
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_mmap: Invalid access\n");
            return -EACCES; 
		}

		/* Refer to the man page of mmap2 */
		if(flags == MAP_SHARED && prot == PROT_WRITE && curproc->p_files[fd]->f_mode != (FMODE_READ || FMODE_WRITE))
		{
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_mmap: Invalid access\n");
            return -EACCES; 
		}

		/* Refer to the man page of mmap2 */
		if(flags == MAP_SHARED && prot == PROT_WRITE && curproc->p_files[fd]->f_mode != FMODE_APPEND)
		{
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_mmap: Invalid access\n");
            return -EACCES; 
		}

		/* Flushing the TLB */
		int address = (uintptr_t)addr;
		tlb_flush(address);

		/* Calling the function vmmmap_map */
		uint32_t lopage = ADDR_TO_PN(addr);
		uint32_t npages = len / PAGE_SIZE + 1;
		int i = vmmap_map(curproc->p_vmmap, curproc->p_files[fd]->f_vnode, lopage, npages, prot, flags, off, VMMAP_DIR_HILO, (vmarea_t**)ret);

		dbg(DBG_USER, "GRADING: KASSERT(NULL != curproc->p_pagedir), I'm going to invoke this assert right now!\n");
		KASSERT(NULL != curproc->p_pagedir);
		dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");

		return 0;

        /*NOT_YET_IMPLEMENTED("VM: do_mmap");
        return -1;*/
}


/*
 * This function implements the munmap(2) syscall.
 *
 * As with do_mmap() it should perform the required error checking,
 * before calling upon vmmap_remove() to do most of the work.
 * Remember to clear the TLB.
 */
int
do_munmap(void *addr, size_t len)
{
		/* Ivalid addr */
		if(PAGE_ALIGNED(addr)==0)
		{
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_munmap: Invalid addr\n");
            return -EINVAL; 
		}

		/* Invalid len, len should be not be NULL */
		if (sizeof(len) == NULL)
		{
			dbg(DBG_ERROR | DBG_VM, "ERROR: do_munmap: Invalid prot\n");
            return -EINVAL; 

		}

		/* Flushing the TLB */
		int address = (uintptr_t)addr;
		tlb_flush(address);

		/* Calling the function vmmap_remove */
		uint32_t lopage = ADDR_TO_PN(addr);
		uint32_t npages = len / PAGE_SIZE + 1;
		int i = vmmap_remove(curproc->p_vmmap, lopage, npages);

		dbg(DBG_USER, "GRADING: KASSERT(NULL != curproc->p_pagedir), I'm going to invoke this assert right now!\n");
		KASSERT(NULL != curproc->p_pagedir);
		dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");

		return 0;

        /*NOT_YET_IMPLEMENTED("VM: do_munmap");
        return -1;*/
}

