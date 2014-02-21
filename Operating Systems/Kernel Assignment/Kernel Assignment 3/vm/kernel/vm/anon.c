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

int anon_count = 0; /* for debugging/verification purposes */

static slab_allocator_t *anon_allocator;

static void anon_ref(mmobj_t *o);
static void anon_put(mmobj_t *o);
static int  anon_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  anon_fillpage(mmobj_t *o, pframe_t *pf);
static int  anon_dirtypage(mmobj_t *o, pframe_t *pf);
static int  anon_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t anon_mmobj_ops = {
        .ref = anon_ref,
        .put = anon_put,
        .lookuppage = anon_lookuppage,
        .fillpage  = anon_fillpage,
        .dirtypage = anon_dirtypage,
        .cleanpage = anon_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * anonymous page sub system. Currently it only initializes the
 * anon_allocator object.
 */
 /*work*/
void
anon_init()
{
        
        dbg(DBG_VFS,"VM: Enter anon_init()\n");
        anon_allocator = slab_allocator_create("anon", sizeof(mmobj_t));

        dbg(DBG_USER, "GRADING: KASSERT(anon_allocator), I'm going to invoke this assert right now!\n");
        KASSERT(anon_allocator);
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");

        KASSERT(NULL != anon_allocator && "failed to create anon allocator!");
        dbg(DBG_VFS,"VM: Leave anon_init()\n");
        /*NOT_YET_IMPLEMENTED("VM: anon_init");*/
}

/*
 * You'll want to use the anon_allocator to allocate the mmobj to
 * return, then then initialize it. Take a look in mm/mmobj.h for
 * macros which can be of use here. Make sure your initial
 * reference count is correct.
 */
 /**/
mmobj_t *
anon_create()
{
        
        dbg(DBG_VFS,"VM: Enter anon_create()\n");
        mmobj_t * newanon=(mmobj_t *) slab_obj_alloc(anon_allocator);
        if(newanon)
        {
                mmobj_init(newanon, &anon_mmobj_ops);
                newanon->mmo_un.mmo_vmas=*mmobj_bottom_vmas(newanon);
                newanon->mmo_refcount++;
        }
        dbg(DBG_VFS,"VM: Leave anon_create()\n");
        return newanon;
        /*NOT_YET_IMPLEMENTED("VM: anon_create");
        return NULL;*/
}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
anon_ref(mmobj_t *o)
{
        dbg(DBG_VFS,"VM: Enter anon_ref(), reference_count =%d, nrespages=%d\n", o->mmo_refcount,o->mmo_nrespages);
        dbg(DBG_USER, "GRADING: KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops)), I'm going to invoke this assert right now!\n");
        KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops));
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");
        o->mmo_refcount++;
        dbg(DBG_VFS,"VM: Leave anon_ref(), reference_count =%d, nrespages=%d\n", o->mmo_refcount,o->mmo_nrespages);
        /*NOT_YET_IMPLEMENTED("VM: anon_ref");*/
}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is an anonymous object, it will
 * never be used again. You should unpin and uncache all of the
 * object's pages and then free the object itself.
 */
static void
anon_put(mmobj_t *o)
{
        dbg(DBG_VFS,"VM: Enter anon_put()\n");
        dbg(DBG_USER, "GRADING: KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops)), I'm going to invoke this assert right now!\n");
        KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops)); 
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");      
        o->mmo_refcount--;
        if(o->mmo_refcount > o->mmo_nrespages)
        {
                dbg(DBG_VFS,"VM: Leave anon_put(), o->mmo_refcount > o->mmo_nrespages\n");
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
                        slab_obj_free(anon_allocator, o);
                }
        }
        dbg(DBG_VFS,"VM: Leave anon_put()\n");
        /*NOT_YET_IMPLEMENTED("VM: anon_put");*/
}

/* Get the corresponding page from the mmobj. No special handling is
 * required. 
         *Finds the correct page frame from a high-level perspective
         * for performing the given operation on an area backed by
         * the given pagenum of the given object. If "forwrite" is
         * specified then the pframe should be suitable for writing;
         * otherwise, it is permitted not to support writes. In
         * either case, it must correctly support reads.
         *
         * Most objects will simply return a page from their
         * own list of pages, but objects such as shadow objects
         * may need to perform more complicated operations to find
         * the appropriate page.
         * This may block.
         * Return 0 on success and -errno otherwise. 
         */
 

static int
anon_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
{
        dbg(DBG_VFS,"VM: Enter anon_lookuppage()\n");
        pframe_t *pframe=pframe_get_resident(o,pagenum);
        if(pframe)
        {
                if(pframe_is_busy(pframe))
                {
                        sched_sleep_on(&pframe->pf_waitq);
                }
                *pf=pframe;
                dbg(DBG_VFS,"VM: Leave anon_lookuppage(), success\n");
                return 0;
        }
        dbg(DBG_VFS,"VM: Leave anon_lookuppage(), error\n");
        return -1;
        /*NOT_YET_IMPLEMENTED("VM: anon_lookuppage");
        return -1;*/
}

/* The following three functions should not be difficult. */

/* You need to remember that you are dealing with polymorphic code here.  
   Since these functions are needed for other memory objects, you MUST 
   implement them for anonymous objects.  If there's nothing to do for 
   anonymous object, then do nothing!

   One comment about your last question...  Anonymous object points to 
   a page of zeros.  I'm not sure why you said that it is not associated a 
   physical page.  (Of course, you can never modify the memory associated 
   with an anonymous object.  So, you cannot dirty the page.)
*/
static int
anon_fillpage(mmobj_t *o, pframe_t *pf)
{
        dbg(DBG_USER, "GRADING: KASSERT(pframe_is_busy(pf)), I'm going to invoke this assert right now!\n");
        KASSERT(pframe_is_busy(pf));
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");  
        dbg(DBG_USER, "GRADING: KASSERT(!pframe_is_pinned(pf)), I'm going to invoke this assert right now!\n");  
        KASSERT(!pframe_is_pinned(pf));
        dbg(DBG_USER, "GRADING: I've made it!  May I have 2 points please!\n");

        KASSERT(pf != NULL);

        dbg(DBG_VFS,"VM: Enter anon_fillpage()\n");

        memset(pf->pf_addr, 0, PAGE_SIZE);
        dbg_print("VM: In anon_fillpage(), pf->pf_addr=0x%x, memset success\n", (uint32_t)pf->pf_addr);

        if(!pframe_is_pinned(pf))
        {
                pframe_pin(pf);
        }

        dbg(DBG_VFS,"VM: Enter anon_fillpage()\n");
        return 0;
}

static int
anon_dirtypage(mmobj_t *o, pframe_t *pf)
{

        /*NOT_YET_IMPLEMENTED("VM: anon_dirtypage");*/
        return -1;
}

static int
anon_cleanpage(mmobj_t *o, pframe_t *pf)
{
        
        /*NOT_YET_IMPLEMENTED("VM: anon_cleanpage");*/
        return -1;
}
