#include "types.h"
#include "globals.h"
#include "kernel.h"
#include "errno.h"

#include "util/debug.h"

#include "proc/proc.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/pagetable.h"

#include "vm/pagefault.h"
#include "vm/vmmap.h"

/*
 * This gets called by _pt_fault_handler in mm/pagetable.c The
 * calling function has already done a lot of error checking for
 * us. In particular it has checked that we are not page faulting
 * while in kernel mode. Make sure you understand why an
 * unexpected page fault in kernel mode is bad in Weenix. You
 * should probably read the _pt_fault_handler function to get a
 * sense of what it is doing.
 *
 * Before you can do anything you need to find the vmarea that
 * contains the address that was faulted on. Make sure to check
 * the permissions on the area to see if the process has
 * permission to do [cause]. If either of these checks does not
 * pass kill the offending process, setting its exit status to
 * EFAULT (normally we would send the SIGSEGV signal, however
 * Weenix does not support signals).
 *
 * Now it is time to find the correct page (don't forget
 * about shadow objects, especially copy-on-write magic!). Make
 * sure that if the user writes to the page it will be handled
 * correctly.
 *
 * Finally call pt_map to have the new mapping placed into the
 * appropriate page table.
 *
 * @param vaddr the address that was accessed to cause the fault
 *
 * @param cause this is the type of operation on the memory
 *              address which caused the fault, possible values
 *              can be found in pagefault.h
 */
 
void
handle_pagefault(uintptr_t vaddr, uint32_t cause)
{
	/*vaddr = (uintptr_t)0xc0001000;*/
	dbg(DBG_VFS,"VM: Enter handle_pagefault(), cause=%d, vaddr=0x%x, pagenum_x=0x%x,pagenum_d=%d\n", cause, vaddr, ADDR_TO_PN(vaddr),ADDR_TO_PN(vaddr));


	/*char buffer[1024];
    	pt_mapping_info(curproc->p_pagedir, buffer, 1024);
     	dbg_print("Page table info:\nVritual Address --> Physical Address\n%s\n", buffer);*/
	vmarea_t *fault_vma=vmmap_lookup(curproc->p_vmmap, ADDR_TO_PN(vaddr));
	/*find vmarea*/
	if(fault_vma==NULL)
	{
		dbg(DBG_VFS,"VM: In handle_pagefault()， fault_vma=NULL\n");
		dbg(DBG_VFS,"VM: In handle_pagefault(), kill curproc id=%d\n", curproc->p_pid);
		proc_kill(curproc, -EFAULT);
		dbg(DBG_VFS,"VM: Leave handle_pagefault(), NULL\n");
		return;
	}/*check permission*/
	else if(cause&FAULT_RESERVED)
	{
		dbg(DBG_VFS,"VM: In handle_pagefault()， check FAULT_RESERVED\n");
		if(!fault_vma->vma_prot&PROT_NONE)
		{
			proc_kill(curproc, -EFAULT);
			dbg(DBG_VFS,"VM: Leave handle_pagefault(), FAULT_RESERVED\n");
			return;
		}
	}
	else if(cause&FAULT_EXEC)
	{
		dbg(DBG_VFS,"VM: In handle_pagefault()， check FAULT_EXEC\n");
		if(!(fault_vma->vma_prot&PROT_EXEC))
		{
			proc_kill(curproc, -EFAULT);
			dbg(DBG_VFS,"VM: Leave handle_pagefault(), FAULT_EXEC\n");
			return;
		}
	}
	else if(cause&FAULT_WRITE)
	{
		dbg(DBG_VFS,"VM: In handle_pagefault()， check FAULT_WRITE\n");
		if(!(fault_vma->vma_prot & PROT_WRITE))
		{
			dbg(DBG_VFS,"VM: In handle_pagefault()， FAULT_WRITE, proc kill\n");
			proc_kill(curproc, -EFAULT);
			dbg(DBG_VFS,"VM: Leave handle_pagefault(), FAULT_WRITE\n");
			return;
		}
	}
	else if(cause & FAULT_PRESENT)
	{
		dbg(DBG_VFS,"VM: In handle_pagefault()， check FAULT_PRESENT\n");
		if(!(fault_vma->vma_prot & PROT_READ))
		{
			proc_kill(curproc, -EFAULT);
			dbg(DBG_VFS,"VM: Leave handle_pagefault(), FAULT_WRITE\n");
			return;
		}
	}
	/*to find the correct page*/
	pframe_t *result_pframe=NULL;
	
	
	/* dbg(DBG_VFS,"VM: before pframe_get\n result_pframe->pf_addr=0x%x\n page_align_up=0x%x\n page_align_down=0x%x, page_offset=0x%x\n", result_pframe->pf_addr,  PAGE_ALIGN_UP(result_pframe->pf_addr), PAGE_ALIGN_DOWN(result_pframe->pf_addr),  PAGE_OFFSET(result_pframe->pf_addr)); */
	/* if(fault_vma->vma_flags==MAP_PRIVATE && fault_vma->vma_obj->mmo_shadowed!=NULL) */
	dbg(DBG_VFS,"VM: vma_flags: %d\n", fault_vma->vma_flags);

	if(fault_vma->vma_flags&MAP_PRIVATE )
	{
		dbg_print("VM: MAP_PRIVATE, pframe_get\n");
		/*pframe_get(fault_vma->vma_obj->mmo_shadowed,ADDR_TO_PN(vaddr),&result_pframe);*/
		pframe_get(fault_vma->vma_obj,PAGE_OFFSET(ADDR_TO_PN(vaddr)),&result_pframe);
		/*dbg_print("VM: In handle_pagefault(), after pframe_get\n");*/	

		/*
		fault_vma->vma_obj->mmo_shadowed->mmo_ops->lookuppage(fault_vma->vma_obj->mmo_shadowed,ADDR_TO_PN(vaddr),cause&FAULT_WRITE,&result_pframe);
		*/
	}
	else if(fault_vma->vma_flags&MAP_SHARED)
	{
		dbg_print("VM: MAP_SHARED, pframe_get\n");
		pframe_get(fault_vma->vma_obj,PAGE_OFFSET(ADDR_TO_PN(vaddr)),&result_pframe);
		dbg_print("VM: In handle_pagefault(), after pframe_get\n");
		/*
		fault_vma->vma_obj->mmo_ops->lookuppage(fault_vma->vma_obj,ADDR_TO_PN(vaddr),cause&FAULT_WRITE,&result_pframe);
		*/
	}
	uint32_t pdflags=FAULT_USER;
	uint32_t ptflags=FAULT_USER;
	if(cause & FAULT_PRESENT)
	{
		ptflags=ptflags|PT_PRESENT;
		pdflags=pdflags|PD_PRESENT;
	}
	if(cause & FAULT_WRITE)
	{
		ptflags=ptflags|PT_WRITE;
		pdflags=pdflags|PD_WRITE;
	}
	
	/*uintptr_t paddr = (uint32_t)result_pframe->pf_addr;
	
	pt_map( pt_get(),(uint32_t)PAGE_ALIGN_UP(vaddr),(uint32_t)PAGE_ALIGN_UP(paddr) ,PROT_WRITE|PROT_READ|PROT_EXEC, PROT_WRITE|PROT_READ|PROT_EXEC);*/


	/* dbg_print("VM: In handle_pagefault(), pt_map\n"); */

	/*uintptr_t paddr = (uint32_t)result_pframe->pf_addr;*/

	/*
	pt_map(curproc->p_pagedir,(uint32_t)PAGE_ALIGN_DOWN(vaddr),(uint32_t)PAGE_ALIGN_DOWN((uint32_t)paddr),PROT_WRITE|PROT_READ|PROT_EXEC, PROT_WRITE|PROT_READ|PROT_EXEC);
	*/
	dbg(DBG_VFS,"VM: after pframe_get, result_pframe->pf_addr=0x%x\n", (uint32_t)result_pframe->pf_addr);
	
	uintptr_t paddr = pt_virt_to_phys((uint32_t)result_pframe->pf_addr);
	

	/* dbg(DBG_VFS,"VM: before pt_map(), vaddr:0x%x, paddr:0x%x, pdflags:%d, ptflags:%d\n",(uint32_t)PAGE_ALIGN_UP(vaddr), (uint32_t)PAGE_ALIGN_UP(paddr), pdflags, ptflags); */
	/* char buffer[1024]; */
    	

	pt_map(curproc->p_pagedir,(uint32_t)PAGE_ALIGN_DOWN(vaddr),(uint32_t)PAGE_ALIGN_DOWN((uint32_t)paddr),PROT_WRITE|PROT_READ|PROT_EXEC, PROT_WRITE|PROT_READ|PROT_EXEC);
	dbg(DBG_VFS,"VM: after pframe_get\n");
	/*pt_mapping_info(curproc->p_pagedir, buffer, 1024);
     	dbg_print("Page table info:\nVritual Address --> Physical Address\n%s\n", buffer);*/

	dbg_print("VM: Leave handle_pagefault()\n");
    /*NOT_YET_IMPLEMENTED("VM: handle_pagefault");*/
}
