/*
 *  FILE: open.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Mon Apr  6 19:27:49 1998
 */

#include "globals.h"
#include "errno.h"
#include "fs/fcntl.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/stat.h"
#include "util/debug.h"

/* find empty index in p->p_files[] */
int
get_empty_fd(proc_t *p)
{
        int fd;

        for (fd = 0; fd < NFILES; fd++) {
                if (!p->p_files[fd])
                        return fd;
        }

        dbg(DBG_ERROR | DBG_VFS, "ERROR: get_empty_fd: out of file descriptors "
            "for pid %d\n", curproc->p_pid);
        return -EMFILE;
}

/*
 * There a number of steps to opening a file:
 *      1. Get the next empty file descriptor.
 *      2. Call fget to get a fresh file_t.
 *      3. Save the file_t in curproc's file descriptor table.
 *      4. Set file_t->f_mode to OR of FMODE_(READ|WRITE|APPEND) based on
 *         oflags, which can be O_RDONLY, O_WRONLY or O_RDWR, possibly OR'd with
 *         O_APPEND.
 *      5. Use open_namev() to get the vnode for the file_t.
 *      6. Fill in the fields of the file_t.
 *      7. Return new fd.
 *
 * If anything goes wrong at any point (specifically if the call to open_namev
 * fails), be sure to remove the fd from curproc, fput the file_t and return an
 * error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        oflags is not valid.
 *      o EMFILE
 *        The process already has the maximum number of files open.
 *      o ENOMEM
 *        Insufficient kernel memory was available.
 *      o ENAMETOOLONG
 *        A component of filename was too long.
 *      o ENOENT
 *        O_CREAT is not set and the named file does not exist.  Or, a
 *        directory component in pathname does not exist.
 *      o EISDIR
 *        pathname refers to a directory and the access requested involved
 *        writing (that is, O_WRONLY or O_RDWR is set).
 *      o ENXIO
 *        pathname refers to a device special file and no corresponding device
 *        exists.
 */


int
do_open(const char *filename, int oflags)
{
    dbg(DBG_VFS,"VFS: Enter do_open(), filename %s\n",filename);
    /* get the least significant bit of the oflags, and validate the oflags.
     * O_WRONLY, O_RDONLY, O_RDWR is mutual exculsive and at least one of 
     * them should be included in oflags.
     */
    int flag = oflags & 0x003;
    dbg(DBG_VFS, "VFS: In do_open(), flag=%x\n", flag);
    if (flag == 3)
    {
        dbg(DBG_VFS,"VFS: Exit do_open(), not a valid mode\n");
        return -EINVAL;
    }
            /*-- 1. Get the next empty file descriptor --*/
    int fd = get_empty_fd(curproc);
    if (fd == -EMFILE)
    {
        dbg(DBG_VFS,"VFS: Exit do_open(), -EMFILE;\n");
        return -EMFILE;
    }
         
    /*-- 2. Call fget to get a fresh file_t --*/
    file_t *f = fget(-1);
    if (f == NULL)
    {
        dbg(DBG_VFS,"VFS: Exit do_open(), f == NULL;\n");
        return -ENOMEM;
    }
        
    /*-- 3. Save the file_t in curproc's file descriptor table --*/
    KASSERT(!curproc->p_files[fd]);
    curproc->p_files[fd] = f;
    
    /*-- 4. Set file_t->f_mode to OR of FMODE_(READ|WRITE|APPEND) based on oflags --*/
    switch(oflags&0x40f)
    {
        case O_RDONLY:
            f->f_mode = FMODE_READ;
            break;
        case O_WRONLY:
            f->f_mode = FMODE_WRITE;
            break;
        case O_RDWR:
            f->f_mode = FMODE_READ | FMODE_WRITE;
            break;
        case (O_RDONLY | O_APPEND):
            f->f_mode = FMODE_READ | FMODE_APPEND;
            break;
        case (O_WRONLY | O_APPEND):
            f->f_mode = FMODE_WRITE | FMODE_APPEND;
            break;
        case (O_RDWR | O_APPEND):
            f->f_mode = FMODE_READ | FMODE_WRITE | FMODE_APPEND;
            break;
        default:
            break;      
    }

    dbg_print("In do_open(), oflags=%x\n", oflags);
    dbg_print("In do_open(), f->f_mode=%x\n", f->f_mode);
    
    /* 5. Use open_namev() to get the vnode for the file_t; 
     * if fails, remove the fd from curproc, fput the file_t
     * and return an error. 
     */
    vnode_t *res_vnode;
    int error;
    int create=0;
    if ((oflags & O_CREAT))
    {
        create=1;
    }
    if((error = open_namev(filename, create, &res_vnode, NULL)) != 0 )  
    {
        curproc->p_files[fd] = NULL;
        fput(f);
        dbg(DBG_VFS,"VFS: Exit do_open(), file not exists;\n");
        return error;
    }

    /* check if is writing to a directory */
    if (S_ISDIR(res_vnode->vn_mode) && (oflags & O_WRONLY || oflags & O_RDWR) )
    {
        curproc->p_files[fd] = NULL;
        fput(f);
    vput(res_vnode);
        dbg(DBG_VFS,"VFS: Exit do_open(), -EISDIR;");
        return -EISDIR;
    }
    /* pathname refers to a device special file and no corresponding device exists */
    if(S_ISCHR(res_vnode->vn_mode))
    {
        if(!(res_vnode->vn_cdev = bytedev_lookup(res_vnode->vn_devid)))
        {
            curproc->p_files[fd] = NULL;
            fput(f);
        vput(res_vnode);
            dbg(DBG_VFS,"VFS: Exit do_open(), -ENXIO 1;");
            return -ENXIO;
        }
    }
    if(S_ISBLK(res_vnode->vn_mode))
    {
        if(!(res_vnode->vn_bdev = blockdev_lookup(res_vnode->vn_devid)))
        {
            curproc->p_files[fd] = NULL;
            fput(f);
        vput(res_vnode);
            dbg(DBG_VFS,"VFS: Exit do_open(), -ENXIO 2;");
            return -ENXIO;
        }
    }

    /*-- 6. Fill in the fields of the file_t --*/
    f->f_vnode = res_vnode;
    dbg_print("VFS: In do_open() filename=%s, f->ref=%d, fd=%d,\nf->vno=%d, f->vno->ref=%d\n", 
        filename, f->f_refcount, fd, f->f_vnode->vn_vno, f->f_vnode->vn_refcount);
    f->f_pos = 0;

    /* check O_TRUNC:
     * If the file already exists and is a regular file and the open mode 
     * allows writing (i.e., is O_RDWR or O_WRONLY) it will be truncated to
     * length 0. If the file is a FIFO or terminal device file, the O_TRUNC
     * flag is ignored. Otherwise the effect of O_TRUNC is unspecified.
     */
    /*if ( (oflags & O_TRUNC & O_WRONLY) || (oflags & O_TRUNC & O_RDWR) ) && S_ISREG(res_vnode->vn_mode) )
    {

    }*/
    dbg(DBG_VFS,"VFS: Leave do_open(),success return fd=%d\n", fd);
    return fd;
}
