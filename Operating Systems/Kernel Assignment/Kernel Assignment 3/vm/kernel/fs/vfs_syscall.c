/*
 *  FILE: vfs_syscall.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Wed Apr  8 02:46:19 1998
 *  $Id: vfs_syscall.c,v 1.1 2012/10/10 20:06:46 william Exp $
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"
 
/* To read a file:
 *      o fget(fd)
 *      o call its virtual read f_op
 *      o update f_pos
 *      o fput() it
 *      o return the number of bytes read, or an error
 * 
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for reading.
 *      o EISDIR
 *        fd refers to a directory.
 *
 * In all cases, be sure you do not leak file refcounts by returning before
 * you fput() a file that you fget()'ed.
 */
int
do_read(int fd, void *buf, size_t nbytes)
{
        if(fd < 0 || fd > NFILES)
            return -EBADF;
        dbg(DBG_VFS,"VFS: Enter do_read()\n");
        file_t* file=fget(fd);
        if(file==NULL)
        {
                dbg(DBG_VFS,"VFS: Leave do_read(), fd is not a valid file descriptor, throw EBADF\n");
                return -EBADF;
        }
        if(!(file->f_mode&FMODE_READ))
        {
                fput(file);
                dbg(DBG_VFS,"VFS: Leave do_read(), fd is not open for reading, throw EBADF\n");
                return -EBADF;
        }
        if(S_ISDIR(file->f_vnode->vn_mode)||file->f_vnode->vn_ops->read==NULL)
        {
                fput(file);
                dbg(DBG_VFS,"VFS: Leave do_read(), fd refers to a directory, throw EISDIR.\n");
                return -EISDIR;
        }
        if(nbytes==0)
        {
            fput(file);
            return 0;
        }
        unsigned int bytes=file->f_vnode->vn_ops->read(file->f_vnode,file->f_pos,buf,nbytes);
        if(bytes==nbytes)
        {
            do_lseek(fd,bytes,SEEK_CUR); 
        }
        else
        {
            do_lseek(fd,0,SEEK_END);
        }
        fput(file);
        dbg(DBG_VFS,"VFS: Leave do_read(), success, return %d.\n",bytes);
        return bytes;
}  

/* Very similar to do_read.  Check f_mode to be sure the file is writable.  If
 * f_mode & FMODE_APPEND, do_lseek() to the end of the file, call the write
 * f_op, and fput the file.  As always, be mindful of refcount leaks.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for writing.
 */
int
do_write(int fd, const void *buf, size_t nbytes)
{
        if(fd < 0 || fd > NFILES)
            return -EBADF;
        dbg_print("VFS: Enter do_write(), fd=%d\n", fd);
        file_t* file=fget(fd);
        if(file==NULL)
        {
                dbg(DBG_VFS,"VFS: Leave do_write(), file=NULL\n");
                return -EBADF;
        }

        if(!(file->f_mode&FMODE_WRITE)&&!(file->f_mode&FMODE_APPEND))
        {
                fput(file);
                dbg(DBG_VFS,"VFS: Leave do_write(), is not open for writing, throw EBADF\n");
                return -EBADF;
        }
        if(S_ISDIR(file->f_vnode->vn_mode)||file->f_vnode->vn_ops->write==NULL)
        {
                fput(file);
                dbg(DBG_VFS,"VFS: Leave do_write(), file is a directory, throw EISDIR \n");
                return -EISDIR;
        }

        int bytes;
        if(file->f_mode&FMODE_APPEND)
        {
                do_lseek(fd,0,SEEK_END);
                KASSERT((S_ISCHR(file->f_vnode->vn_mode)) ||
                        (S_ISBLK(file->f_vnode->vn_mode)) ||
                        ((S_ISREG(file->f_vnode->vn_mode)) && (file->f_pos <= file->f_vnode->vn_len)));
                bytes=file->f_vnode->vn_ops->write(file->f_vnode,file->f_pos,buf,nbytes);
                do_lseek(fd,0,SEEK_END);
                dbg(DBG_VFS,"VFS: Leave do_write(), success, return %d\n",bytes);
        }
        else if(file->f_mode&FMODE_WRITE)
        {
                bytes=file->f_vnode->vn_ops->write(file->f_vnode,file->f_pos,buf,nbytes);  
                do_lseek(fd,bytes,SEEK_CUR);   
                KASSERT((S_ISCHR(file->f_vnode->vn_mode)) ||
                        (S_ISBLK(file->f_vnode->vn_mode)) ||
                        ((S_ISREG(file->f_vnode->vn_mode)) && (file->f_pos <= file->f_vnode->vn_len)));
                dbg(DBG_VFS,"VFS: Leave do_write(), success, return %d\n",bytes); 
        }
        fput(file);
        return bytes;
}

/*
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't a valid open file descriptor.
 */
int
do_close(int fd)
{
        dbg(DBG_VFS,"VFS: Enter do_close(), fd=%d\n", fd);
        if(fd < 0 || fd > NFILES)
            return -EBADF;
        file_t* file=fget(fd);
        if(file==NULL)
        {
                dbg(DBG_VFS,"VFS: Leave do_close(), error, fd isn't a valid open file descriptor, fd=%d\n", fd);
                return -EBADF;
        }
       fput(file);
        /*while(file->f_refcount!=0)*/
        {
                fput(file);
        }
        curproc->p_files[fd]=NULL;
        dbg(DBG_VFS,"VFS: Leave do_close(), success, return 0\n");
        return 0;
}

/* To dup a file:
 *      o fget(fd) to up fd's refcount
 *      o get_empty_fd()
 *      o point the new fd to the same file_t* as the given fd
 *      o return the new file descriptor
 *
 * Don't fput() the fd unless something goes wrong.  Since we are creating
 * another reference to the file_t*, we want to up the refcount.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't an open file descriptor.
 *      o EMFILE
 *        The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int
do_dup(int fd)
{
        dbg(DBG_VFS,"VFS: Enter do_dup().\n");
        if(fd < 0 || fd > NFILES)
            return -EBADF;
        file_t* file=fget(fd);
        if(file==NULL)
        {
                dbg(DBG_VFS,"VFS: Leave do_dup(),fd isn't an open file descriptor\n");
                return -EBADF;
        }
        int new_fd=get_empty_fd(curproc);
        if(new_fd==-EMFILE)
        {
                fput(file);
                dbg(DBG_VFS,"VFS: Leave do_dup(), throw EMFILE\n");
                return -EMFILE;
        }
        curproc->p_files[new_fd]=file;
        /*vget(file->f_vnode->vn_fs,file->f_vnode->vn_vno);*/
        dbg(DBG_VFS,"VFS: Leave do_dup()\n");

        file_t* nfile=fget(new_fd);
        fput(nfile);
        dbg(DBG_VFS,"VFS: Leave do_dup(), New fd=%d, vnode %d's reference count %d\n", 
            new_fd, nfile->f_vnode->vn_vno, nfile->f_vnode->vn_refcount);

        return new_fd;
}

/* Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first.  Then return the new file descriptor.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int
do_dup2(int ofd, int nfd)
{
        if(ofd < 0 || ofd > NFILES)
            return -EBADF;
        dbg(DBG_VFS,"VFS: Enter do_dup2()\n");
        file_t* file=fget(ofd);
        if(file==NULL)
        {
                dbg(DBG_VFS,"VFS: Leave do_dup2(), file is NULL, throw EBADF.\n");
                return -EBADF;
        }
        if(nfd>NFILES||nfd<0)
        {
                fput(file);
                dbg(DBG_VFS,"VFS: Leave do_dup2(), nfd is invalid, throw EBADF.\n");
                return -EBADF;
        }
        if(curproc->p_files[nfd]!=NULL&&nfd!=ofd)
        {
                do_close(nfd);
        }
        if(nfd==ofd)
        {
            dbg(DBG_VFS,"VFS: Leave do_dup2(), nfd and old is the same.\n");
            fput(file);
            return nfd;
        }
        curproc->p_files[nfd]=file;
        /*vget(file->f_vnode->vn_fs,file->f_vnode->vn_vno);*/
        file_t* nfile=fget(nfd);
        fput(nfile);
        dbg(DBG_VFS,"VFS: Leave do_dup2(), nfd=%d, vnode %d's reference count %d\n", 
            nfd, nfile->f_vnode->vn_vno, nfile->f_vnode->vn_refcount);
        return nfd;
}

/*
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'. 'mode' should be one of S_IFCHR or
 * S_IFBLK (you might note that mknod(2) normally allows one to create
 * regular files as well-- for simplicity this is not the case in Weenix).
 * 'devid', as you might expect, is the device identifier of the device
 * that the new special file should represent.
 *
 * You might use a combination of dir_namev, lookup, and the fs-specific
 * mknod (that is, the containing directory's 'mknod' vnode operation).
 * Return the result of the fs-specific mknod, or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        mode requested creation of something other than a device special
 *        file.
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
/* make block or character special files 
  * S_IFCHR: character special file
  * S_IFBLK: block special file
  * 
  */
int
do_mknod(const char *path, int mode, unsigned devid)
{
        dbg(DBG_VFS,"VFS: Enter do_mknod(), path %s\n", path);
        /* validate mode type */
        if (!(S_ISCHR(mode) || (S_ISBLK(mode))))
        {
                dbg(DBG_VFS,"VFS: Leave do_mknod(), mode error\n");
                return -EINVAL;
        }
        /* get directory vnode*/
        size_t namelen;
        const char *name;
        vnode_t *dir;
        int error;
        if((error = dir_namev(path, &namelen, &name, NULL, &dir) ) != 0) /*ENOENT ENOTDIR ENAMETOOLONG*/
        {
                dbg(DBG_VFS,"VFS: Leave do_mknod(), error cannot find dir\n");
                return error;
        }
        vnode_t *result;
        if ((error = lookup(dir, name, namelen, &result)) != 0)
        {
                /* check if result vnode is a directory vnode*/
                if (error == -ENOTDIR || dir->vn_ops->mknod==NULL||!S_ISDIR(dir->vn_mode))/* ?bug fixed here*/
                {
                        vput(dir);
                        dbg(DBG_VFS,"VFS: Leave do_mknod(), error is not dir, throw ENOTDIR\n");
                        return -ENOTDIR;
                }
                KASSERT(S_ISDIR(dir->vn_mode));
                /* path doesn't exist */
                if (error == -ENOENT)
                {
                        KASSERT(NULL != dir->vn_ops->mknod);
                        int ret = dir->vn_ops->mknod(dir, name, namelen, mode, (devid_t)devid);
                        vput(dir);
                        dbg(DBG_VFS,"VFS: Leave do_mknod(), error cannot find name, throw ENOENT\n");
                        return ret;
                }
                vput(dir);
                dbg(DBG_VFS,"VFS: Leave do_mknod(), other error\n");
                return error;
        }
        vput(dir);
        /* vput(result); */
        dbg(DBG_VFS,"VFS: Leave do_mknod(), node already exist.\n");
        return -EEXIST;
}

/* Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. Return what it returns.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mkdir(const char *path)
{
        dbg(DBG_VFS,"VFS: Enter do_mkdir(), path=%s\n", path);
        size_t namelen;
        const char *name;
        vnode_t *dir_vnode;
        vnode_t *res_vnode;
        int err;
        /*Use dir_namev() to find the vnode*/
        /* Err includeing, ENOENT, ENOTDIR, ENAMETOOLONG*/
        if((err = dir_namev(path, &namelen, &name, NULL, &dir_vnode) ) != 0) {
                /* vput(dir_vnode); */
                dbg(DBG_VFS,"VFS: Leave do_mkdir(), cannot find path %s, err=%d\n", path, err);
                return err;
        }
        /* Use lookup() to make sure it doesn't already exist.*/
        if(lookup(dir_vnode, name, namelen, &res_vnode) == 0) {
                vput(dir_vnode);
                vput(res_vnode);
                dbg(DBG_VFS,"VFS: Leave do_mkdir(), err file already exist\n");
                return -EEXIST;
        }
        if(dir_vnode -> vn_ops -> mkdir == NULL) {
            vput(dir_vnode);
            dbg(DBG_VFS,"VFS: Leave do_mkdir(), err not directory\n");
            return -ENOTDIR;
        }
        /* Call the dir's mkdir vn_ops. Return what it returns.*/
        KASSERT(NULL != dir_vnode->vn_ops->mkdir);
        dbg(DBG_VFS,"VFS:In do_mkdir(), before ramfs_mkdir path=%s\n", path);
        err = dir_vnode -> vn_ops -> mkdir(dir_vnode, name, namelen);
        vput(dir_vnode);
        dbg(DBG_VFS,"VFS: Leave do_mkdir(), err=%d\n", err);
        return err;
}

/* Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.  The rmdir v_op will
 * return an error if the dir to be removed does not exist or is not empty, so
 * you don't need to worry about that here. Return the value of the v_op,
 * or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        path has "." as its final component.
 *      o ENOTEMPTY
 *        path has ".." as its final component.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_rmdir(const char *path)
{
        dbg(DBG_VFS,"VFS: Enter do_rmdir(), path=%s\n", path);
        size_t namelen;
        const char *name;
        vnode_t *dir_vnode;
        int err;
        unsigned len = 0;
        /* Check whether path has ".", ".." as its final component. */
        while(path[len++] != '\0');
        len -= 2;
        if(name_match(".", path, len+1))
            return -EINVAL;
        if(name_match("..", path, len+1))
            return -ENOTEMPTY;

        /* Use dir_namev() to find the vnode of the directory containing the dir to be removed. */
        /* Err includeing, ENOENT, ENOTDIR, ENAMETOOLONG */
      
        if((err = dir_namev(path, &namelen, &name, NULL, &dir_vnode) ) != 0) {
                /*vput(dir_vnode);*/
                dbg(DBG_VFS,"VFS: Leave do_rmdir()\n");
                return err;
        }

        if(path[len] == '.') {
                if(path[--len] == '/') {
                        vput(dir_vnode);
                        return -EINVAL;
                    }
                else if(path[len] == '.')
                        if(path[--len] == '/') {
                                vput(dir_vnode);
                                return -ENOTEMPTY;
                            }
        }
        if(dir_vnode -> vn_ops -> rmdir == NULL) {
            vput(dir_vnode);
            dbg(DBG_VFS,"VFS: Leave do_rmdir(), throw ENOTDIR\n");
            return -ENOTDIR;
        }
        /* Call the containing dir's rmdir v_op. */
        KASSERT(NULL != dir_vnode->vn_ops->rmdir);
        err = dir_vnode -> vn_ops -> rmdir(dir_vnode, name, namelen);
        vput(dir_vnode);
        /* Need vput()? */
        dbg(DBG_VFS,"VFS: Leave do_rmdir()\n");
        return err;
}

/*
 * Same as do_rmdir, but for files.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EISDIR
 *        path refers to a directory.
 *      o ENOENT
 *        A component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */

/*
 * unlink removes the link to the vnode in dir specified by name
 */
int
do_unlink(const char *path)
{

        dbg(DBG_VFS,"VFS: Enter do_unlink()\n");
        /* get directory vnode*/
        size_t namelen;
        const char *name;
        vnode_t *dir;
        int error;
        if ((error = dir_namev(path, &namelen, &name, NULL, &dir) ) != 0) 
        {
                dbg(DBG_VFS,"VFS: Leave do_unlink(), cannot find path\n");
                return error;
        }
        /* check if path refers to a directory */
        vnode_t *result;
        if ((error = lookup(dir, name, namelen, &result)) != 0)
        {
                vput(dir);
                dbg(DBG_VFS,"VFS: leave do_unlink(), lookup error\n");
                return error;
        }
        dbg(DBG_VFS,"VFS: In do_unlink() result->vn_mode=%x\n", result->vn_mode);
        if (S_ISDIR(result->vn_mode))
        {
                vput(dir);
                vput(result);
                dbg(DBG_VFS,"VFS: leave do_unlink() , throw EISDIR\n");
                return -EISDIR;
        }
        
        /* reomve the result vnode from the directory*/
        KASSERT(NULL != dir->vn_ops->unlink);
        int ret = dir->vn_ops->unlink(dir, name, namelen);
        vput(result);
        vput(dir);
        dbg(DBG_VFS,"VFS: Leave do_unlink(), sucess\n");
        return ret;
}

/* To link:
 *      o open_namev(from)
 *      o dir_namev(to)
 *      o call the destination dir's (to) link vn_ops.
 *      o return the result of link, or an error
 *
 * Remember to vput the vnodes returned from open_namev and dir_namev.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        to already exists.
 *      o ENOENT
 *        A directory component in from or to does not exist.
 *      o ENOTDIR
 *        A component used as a directory in from or to is not, in fact, a
 *        directory.
 *      o ENAMETOOLONG
 *        A component of from or to was too long.
 */

/*link sets up a hard link. it links oldvnode into dir with the
 * specified name.
 */
int
do_link(const char *from, const char *to)
{
        dbg(DBG_VFS,"VFS: Enter do_link()\n");
        /* get 'from' vnode */
        int error;
        vnode_t *from_vnode;
        if ((error = open_namev(from, 0, &from_vnode, NULL))!= 0) /*? flag=0*/
        {
                dbg(DBG_VFS,"VFS: Leave do_link()\n");
                return error;
        }
        /* get 'to' directory vnode*/
        size_t namelen;
        const char *name;
        vnode_t *to_dir;
        if ((error = dir_namev(to, &namelen, &name, NULL, &to_dir)!= 0))
        {
                vput(from_vnode);
                dbg(DBG_VFS,"VFS: Leave do_link(), cannot find dir\n");
                return error;
        }

        /* check if to_vnode has already existed */
        vnode_t *to_vnode;
        if (lookup(to_dir, name, namelen, &to_vnode) == 0)
        {
                vput(from_vnode);
                vput(to_dir);
                vput(to_vnode);
                dbg(DBG_VFS,"VFS: Leave do_link(), throw EEXIST\n");
                return -EEXIST;
        }

        if (to_dir->vn_ops->link == NULL)
        {
                vput(from_vnode);
                vput(to_dir);
                dbg(DBG_VFS,"VFS: Leave do_link(), throw ENOTDIR\n");
                return -ENOTDIR;
        }
        /* call the destination dir's (to) link vn_ops; 'from_vnode' refcount++ in link()*/
        int ret = to_dir->vn_ops->link(from_vnode, to_dir, name, namelen);

        /* vput the vnodes returned from open_namev and dir_namev */
        vput(from_vnode);
        vput(to_dir);
        dbg(DBG_VFS,"VFS: Leave do_link(),success\n");
        return ret;
}

/*      o link newname to oldname
 *      o unlink oldname
 *      o return the value of unlink, or an error
 *
 * Note that this does not provide the same behavior as the
 * Linux system call (if unlink fails then two links to the
 * file could exist).
 */
int
do_rename(const char *oldname, const char *newname)
{
        
         dbg(DBG_VFS,"VFS: Enter do_rename()\n");
         /* link newname to oldname */
         int error;
         if ( (error = do_link(oldname, newname)) != 0)
         {
                dbg(DBG_VFS,"VFS: Leave do_rename()\n");
                return error;
         }
         /* unlink oldname */
        dbg(DBG_VFS,"VFS: Leave do_rename(), success\n");
        return do_unlink(oldname);
}

/* Make the named directory the current process's cwd (current working
 * directory).  Don't forget to down the refcount to the old cwd (vput()) and
 * up the refcount to the new cwd (open_namev() or vget()). Return 0 on
 * success.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        path does not exist.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o ENOTDIR
 *        A component of path is not a directory.
 */
int
do_chdir(const char *path)
{
        /* Get the current process's cwd */
        vnode_t *old_cwd = curproc -> p_cwd;
        dbg(DBG_VFS,"VFS: Enter do_chdir(), old vno=%d, old reference=%d\n", old_cwd -> vn_vno, old_cwd -> vn_refcount);
        /* Down the refcount to the old cwd */
        vnode_t *new_cwd;
        int err;
        /* Up the refcount; flag!?; Err includeing, ENOENT, ENAMETOOLONG */
        if((err = open_namev(path, 0, &new_cwd, NULL) ) != 0) {
            dbg(DBG_VFS,"VFS: Leave do_chdir(), err=%d\n", err);
            return err;
        }
        /* Check error ENOTDIR */
        if(!S_ISDIR(new_cwd->vn_mode))
        {
                vput(new_cwd);
                return -ENOTDIR;
        }
        dbg(DBG_VFS,"VFS: In do_chdir(), new vno=%d, new reference=%d\n", new_cwd -> vn_vno, new_cwd -> vn_refcount);
        /* fput? */
        vput(old_cwd);
        curproc -> p_cwd = new_cwd;
         dbg(DBG_VFS,"VFS: Leave do_chdir(), old vno=%d, old reference=%d\n", old_cwd -> vn_vno, old_cwd -> vn_refcount);
        return 0;
        /* NOT_YET_IMPLEMENTED("VFS: do_chdir"); */
        /* return -1; */
}

/* Call the readdir f_op on the given fd, filling in the given dirent_t*.
 * If the readdir f_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.  You need to increment the
 * file_t's f_pos by this amount.  As always, be aware of refcounts, check
 * the return value of the fget and the virtual function, and be sure the
 * virtual function exists (is not null) before calling it.
 *
 * Return either 0 or sizeof(dirent_t), or -errno.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        Invalid file descriptor fd.
 *      o ENOTDIR
 *        File descriptor does not refer to a directory.
 */
int
do_getdent(int fd, struct dirent *dirp)
{
        dbg(DBG_VFS,"VFS: Enter do_getdent(), fd=%d\n", fd);
        if(fd < 0 || fd > NFILES)
            return -EBADF;
        file_t *file=fget(fd);
        if(file==NULL)
        {
                dbg(DBG_DISK,"VFS: Leave do_getdent(), error EBADF, fd=%d\n", fd);
                return -EBADF;
        }
        if(!S_ISDIR(file->f_vnode->vn_mode)||file->f_vnode->vn_ops->readdir==NULL)
        {
                fput(file);
                dbg(DBG_DISK,"VFS: Leave do_getdent(), error ENOTDIR, fd=%d\n", fd);
                return -ENOTDIR;
        }

        int bytes;
        bytes=file->f_vnode->vn_ops->readdir(file->f_vnode,file->f_pos,dirp);
        /*do_lseek(fd,bytes,SEEK_CUR);*/
        fput(file);
        /*dbg(DBG_DISK,"VFS: Leaving do_getdent()\n");*/
        /*return sizeof(*dirp);*/
        dbg(DBG_VFS,"VFS: In do_getdent(), file->f_pos+bytes=%d\n", file->f_pos+bytes);
        dbg(DBG_VFS,"VFS: In do_getdent(), file->f_vnode->vn_len=%d\n", file->f_vnode->vn_len);
        if(bytes==0)
        {
            do_lseek(fd,0,SEEK_END);
            dbg(DBG_VFS,"VFS: Leave do_getdent(), return 0, fd=%d\n", fd);
            return 0;
        }
        do_lseek(fd,bytes,SEEK_CUR);
        int size = sizeof(*dirp);
        dbg(DBG_VFS,"VFS: Leave do_getdent(), return sizeof(*dirp)=%d, fd=%d\n", size, fd);
        return size;

         /*if(file->f_pos+bytes<file->f_vnode->vn_len)
        {
            file->f_pos+=file->f_pos+bytes;
            return sizeof(*dirp);
        }
        else
        {
            do_lseek(fd,0,SEEK_END);
            return 0;
        }*/
}

/*
 * Modify f_pos according to offset and whence.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not an open file descriptor.
 *      o EINVAL
 *        whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int
do_lseek(int fd, int offset, int whence)
{
        dbg(DBG_VFS,"VFS: Enter do_lseek()\n");
        if(fd < 0 || fd > NFILES)
            return -EBADF;
        if(!(whence==SEEK_SET||whence==SEEK_CUR||whence==SEEK_END))
        {
                dbg(DBG_VFS,"VFS: Leave do_lseek(),mode error \n");
                return -EINVAL;
        }

        file_t *file=fget(fd);
        if(file==NULL)
        {
                dbg(DBG_VFS,"VFS: Leave do_lseek(), file null\n");
                return -EBADF;
        }

        if(whence==SEEK_SET)
        {
            if(offset<0)
            {
                fput(file);
                return -EINVAL;
            }
            else
            {
                file->f_pos=offset;

            }
        }
        else if(whence==SEEK_CUR)
        {
            if(file->f_pos+offset<0)
            {
                fput(file);
                return -EINVAL;
            }
            else
            {
                file->f_pos=file->f_pos+offset;
            }
        }
        else if(whence==SEEK_END)
        {
            if(file->f_vnode->vn_len+offset<0)
            {
                fput(file);
                return -EINVAL;
            }
            else
            {
                file->f_pos=file->f_vnode->vn_len+offset;
            }    
        }
        fput(file);
        dbg(DBG_VFS,"VFS: Leave do_lseek(), success\n");
        return file->f_pos;
}


/*
 * Find the vnode associated with the path, and call the stat() vnode operation.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        A component of path does not exist.
 *      o ENOTDIR
 *        A component of the path prefix of path is not a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_stat(const char *path, struct stat *buf)
{
        dbg(DBG_VFS,"VFS: Enter do_stat(), path=%s\n", path);
        size_t len;
        const char *name;
        vnode_t* par;
        vnode_t* chd;
        int err=0;

        err=dir_namev(path,&len,&name,NULL,&par);
        if(!err)
        {
            dbg(DBG_VFS,"VFS: In do_stat() lookup(),  path=%s, name=%s,len=%d\n", path, name, len);
            err=lookup(par,name,len,&chd);
            if(!err)
            {
                vput(par);
                KASSERT(chd->vn_ops->stat);
                chd->vn_ops->stat(chd,buf);
                vput(chd);
                dbg(DBG_VFS,"VFS: Leave do_stat(), success.\n");
                return 0;
            }
            vput(par);
            dbg(DBG_VFS,"VFS: Leave do_stat(), err1=%d\n", err);
            return err;
        }
        dbg(DBG_VFS,"VFS: Leave do_stat(), err2=%d\n", err);
        return err;
}

#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int
do_mount(const char *source, const char *target, const char *type)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
        return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not worry
 * about freeing the fs_t struct here, that is done in vfs_umount. All this function
 * does is figure out which file system to pass to vfs_umount and do good error
 * checking.
 */
int
do_umount(const char *target)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_umount");
        return -EINVAL;
}
#endif