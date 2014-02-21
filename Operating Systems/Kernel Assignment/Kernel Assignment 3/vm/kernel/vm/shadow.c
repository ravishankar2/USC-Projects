#include "globals.h"
#include "errno.h"

#include "util/string.h"
#include "util/debug.h"

#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/mm.h"
#include "mm/page.h"
#include "mm/slab.h"
#include "mm/tlb.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/shadowd.h"

#define SHADOW_SINGLETON_THRESHOLD 5

int shadow_count = 0; /* for debugging/verification purposes */
#ifdef __SHADOWD__
/*
 * number of shadow objects with a single parent, that is another shadow
 * object in the shadow objects tree(singletons)
 */
static int shadow_singleton_count = 0;
#endif

static slab_allocator_t *shadow_allocator;

static void shadow_ref(mmobj_t *o);
static void shadow_put(mmobj_t *o);
static int  shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  shadow_fillpage(mmobj_t *o, pframe_t *pf);
static int  shadow_dirtypage(mmobj_t *o, pframe_t *pf);
static int  shadow_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t shadow_mmobj_ops = {
        .ref = shadow_ref,
        .put = shadow_put,
        .lookuppage = shadow_lookuppage,
        .fillpage  = shadow_fillpage,
        .dirtypage = shadow_dirtypage,
        .cleanpage = shadow_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * shadow page sub system. Currently it only initializes the
 * shadow_allocator object.
 */
void
shadow_init()
{
        shadow_allocator = slab_allocator_create("shadow", sizeof(mmobj_t));
        dbg(DBG_USER, "GRADING: KASSERT(shadow_allocator), I'm going to invoke this assert right now!\n");
        KASSERT(shadow_allocator);
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");
        /*NOT_YET_IMPLEMENTED("VM: shadow_init");*/
}

/*
 * You'll want to use the shadow_allocator to allocate the mmobj to
 * return, then then initialize it. Take a look in mm/mmobj.h for
 * macros which can be of use here. Make sure your initial
 * reference count is correct.
 */
mmobj_t *
shadow_create()
{
        dbg(DBG_VFS,"VM: Enter shadow_create()\n");
        mmobj_t *shadow_obj = (mmobj_t*)slab_obj_alloc(shadow_allocator);
        if(shadow_obj)
        {
                mmobj_init(shadow_obj,&shadow_mmobj_ops);
                (shadow_obj)->mmo_un.mmo_bottom_obj=mmobj_bottom_obj(shadow_obj);
                shadow_obj->mmo_refcount++;
        }
        dbg(DBG_VFS,"VM: Leave shadow_create()\n");
        return shadow_obj;
        /*NOT_YET_IMPLEMENTED("VM: shadow_create");
        return NULL;*/
}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
shadow_ref(mmobj_t *o)
{
        dbg(DBG_VFS,"VM: Enter shadow_ref(), reference_count =%d, nrespages=%d\n", o->mmo_refcount,o->mmo_nrespages);
        dbg(DBG_USER, "GRADING: KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops)), I'm going to invoke this assert right now!\n");
        KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");
        o->mmo_refcount++;
        dbg(DBG_VFS,"VM: Leave shadow_ref(), reference_count =%d, nrespages=%d\n", o->mmo_refcount,o->mmo_nrespages);
        /*NOT_YET_IMPLEMENTED("VM: shadow_ref");*/
}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is a shadow object, it will never
 * be used again. You should unpin and uncache all of the object's
 * pages and then free the object itself.
 */
static void
shadow_put(mmobj_t *o)
{
        dbg(DBG_USER, "GRADING: KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops)), I'm going to invoke this assert right now!\n");
        KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");

        o->mmo_refcount--;
        if(o->mmo_refcount > o->mmo_nrespages)
        {
                return;
        }
        else
        {
                if(!list_empty(&o->mmo_respages))
                {
                        pframe_t *pf;
                        list_iterate_begin(&(o->mmo_respages), pf, pframe_t,pf_olink)
                        {
                                if(pframe_is_busy(pf))
                                {
                                       sched_sleep_on(&pf->pf_waitq);
                                }
                                while(pframe_is_pinned(pf))
                                {
                                        pframe_unpin(pf);
                                }
                                if(pframe_is_dirty(pf))
                                {
                                        pframe_clean(pf);
                                }
                        }list_iterate_end();
                        /*not sure about this*/
                        pframe_free(pf);
                        slab_obj_free(shadow_allocator, o);
                }
        }
        /*NOT_YET_IMPLEMENTED("VM: shadow_put");*/
}

/* This function looks up the given page in this shadow object. The
 * forwrite argument is true if the page is being looked up for
 * writing, false if it is being looked up for reading. This function
 * must handle all do-not-copy-on-not-write magic (i.e. when forwrite
 * is false find the first shadow object in the chain which has the
 * given page resident). copy-on-write magic (necessary when forwrite
 * is true) is handled in shadow_fillpage, not here. */
static int
shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
{
        pframe_t* result_pframe;
        mmobj_t *entry = o;
        if(!forwrite)
        {
                while((entry->mmo_shadowed)!=NULL)
                {
                        result_pframe=pframe_get_resident(entry,pagenum);
                        if(result_pframe!=NULL)
                                break;
                        entry=entry->mmo_shadowed;
                }
                if(result_pframe!=NULL)
                {
                        if(pframe_is_busy(result_pframe))
                        {
                                sched_sleep_on(&result_pframe->pf_waitq);
                        }
                        *pf=result_pframe;
                        return 0;
                }
                else 
                {
                        entry=mmobj_bottom_obj(o);
                        result_pframe=pframe_get_resident(entry,pagenum);
                        if(result_pframe!=NULL)
                        {
                                if(pframe_is_busy(result_pframe))
                                {
                                        sched_sleep_on(&result_pframe->pf_waitq);
                                }
                                *pf=result_pframe;
                                return 0;
                        }
                        else 
                        {
                                return -1;
                        }
                }
        }
        else
        {
                pframe_get(entry,pagenum,&result_pframe);
                if(result_pframe!=NULL)
                {
                        if(pframe_is_busy(result_pframe))
                        {
                                sched_sleep_on(&result_pframe->pf_waitq);
                        }
                        *pf=result_pframe;
                        return 0;
                }
        }
        return 0;
        /*NOT_YET_IMPLEMENTED("VM: shadow_lookuppage");
        return 0;*/
}

/* As per the specification in mmobj.h, fill the page frame starting
 * at address pf->pf_addr with the contents of the page identified by
 * pf->pf_obj and pf->pf_pagenum. This function handles all
 * copy-on-write magic (i.e. if there is a shadow object which has
 * data for the pf->pf_pagenum-th page then we should take that data,
 * if no such shadow object exists we need to follow the chain of
 * shadow objects all the way to the bottom object and take the data
 * for the pf->pf_pagenum-th page from the last object in the chain). */
static int
shadow_fillpage(mmobj_t *o, pframe_t *pf)
{
        dbg(DBG_USER, "GRADING: KASSERT(pframe_is_busy(pf)), I'm going to invoke this assert right now!\n");
        KASSERT(pframe_is_busy(pf));
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");
        dbg(DBG_USER, "GRADING: KASSERT(!pframe_is_pinned(pf)), I'm going to invoke this assert right now!\n");
        KASSERT(!pframe_is_pinned(pf));
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");
        dbg(DBG_VFS,"Enter shadow_fillpage(), destinaiton object: 0x%p, pf->pf_pagenum: %d\n",o,pf->pf_pagenum);
        pframe_t *pframe;
        /* look for the source page frame */
        int ret = shadow_lookuppage(o->mmo_shadowed,pf->pf_pagenum,0,&pframe);
        if(ret == 0)
        {
                if(pframe)
                {
                        memcpy(pf->pf_addr,pframe->pf_addr,PAGE_SIZE);
                }
                else
                {
                        return -EFAULT;
                }          
             
        }
        else 
                return -1;
        dbg(DBG_VFS,"Fillpage operation completed\n");
        return 0;
        /*NOT_YET_IMPLEMENTED("VM: shadow_fillpage");
        return 0;*/
}

/* These next two functions are not difficult. */

static int
shadow_dirtypage(mmobj_t *o, pframe_t *pf)
{
        if(pframe_is_busy(pf))
        {
               pframe_clear_busy(pf);
               /* broad cast */
        }
        if(!(pframe_is_dirty(pf)))
        {
               pframe_set_dirty(pf);
        }
        KASSERT(pframe_is_dirty(pf));


        return 0;

        /*NOT_YET_IMPLEMENTED("VM: shadow_dirtypage");
        return -1;*/
}

static int
shadow_cleanpage(mmobj_t *o, pframe_t *pf)
{
        pframe_t *pframe;        

        int  ret = shadow_lookuppage(o->mmo_shadowed,pf->pf_pagenum,0,&pframe);

        if(pframe)
        {
                while(pframe_is_pinned(pf))
                pframe_unpin(pf);
                memcpy(pframe,pf,PAGE_SIZE);
                pframe_free(pf);
        }
        else 
                return -1;

        return 0;
        /*NOT_YET_IMPLEMENTED("VM: shadow_cleanpage");
        return -1;*/
}
