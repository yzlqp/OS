/**
 * @file file.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */ 
 
#include "../proc/proc.h"
#include "file.h"
#include "fs/fs.h"
#include "fs/log.h"
#include "../printf.h"
#include "../include/param.h"
#include "../include/stat.h"
#include "../pipe/pipe.h"

struct devsw devsw[NDEV];
struct ftable {
    struct spinlock lock;
    struct file file[NFILE];
} ftable;

/*
 * Initialize the kernel file table
 */
void file_init(void)
{
    init_spin_lock(&ftable.lock, "ftable");
}

/*
 * Allocate a file structure.
 * Scan the Ftable for the first free slot and return the new reference.  
 * When the maximum number of open files is reached, 
 * filealloc simply returns NULL and does not panic.
 */
struct file *filealloc(void)
{
    acquire_spin_lock(&ftable.lock);
    for (struct file *f = ftable.file; f < ftable.file + NFILE; f++) { 
        if (f->ref == 0) { 
            f->ref = 1;
            release_spin_lock(&ftable.lock);
            return f;
        }
    }
    release_spin_lock(&ftable.lock);
    // don't panic when run out of files
    return NULL;
}

/*
 * Create a copy of the open file struct file, just Increment ref count for file f.
 * It then returns a pointer to the same struct file
 */
struct file *filedup(struct file *f)
{
    acquire_spin_lock(&ftable.lock);
    if (f->ref < 1)
        panic("filedup: ref: %d.\n", f->ref);
    f->ref++;
    release_spin_lock(&ftable.lock);
    return f;
}

/*
 * Close specified files (Decrement ref count, close when reaches 0.)
 */
void fileclose(struct file *f)
{
    acquire_spin_lock(&ftable.lock);
    if (f->ref < 1)
        panic("fileclose: ref: %d.\n", f->ref);
    
    if (--f->ref > 0) {
        release_spin_lock(&ftable.lock);
        return;
    }

    struct file ff = *f;
    f->ref = 0;
    f->type = FD_NONE;
    release_spin_lock(&ftable.lock);

    // Releases the underlying pipe or inode, according to the type.
    if (ff.type == FD_PIPE) {
        pipeclose(ff.pipe, ff.writable);
    } else if (ff.type == FD_INODE || ff.type == FD_DEVICE) {
        begin_op();
        iput(ff.ip);
        end_op();
    }
}

/*
 * Get metadata about file f.
 * addr is a user virtual address, pointing to a struct stat.
 * Only inode types are allowed to read their stat information
 * Service the system call fstat
 */
int32_t filestat(struct file *f, struct stat *st)
{
    if (f->type == FD_INODE || f->type == FD_DEVICE) {
        ilock(f->ip);
        stati(f->ip, st);
        iunlock(f->ip);
        return 0;
    }
    return -1;
}

/*
 * Read from file f.
 * addr is a user virtual address.
 * Depending on the underlying file type, check whether file readable mode
 * is turned on, and then call different methods to read these resources, 
 * servicing the system call read.  Fileread and the following FileWrite 
 * use the offset off in struct files, which is updated each time it is 
 * read, with the exception of pipes, which have no offset 
 */
int32_t fileread(struct file *f, char *addr, int32_t n)
{
    int r = 0;

    if(!f->readable) 
        return -1;

    if (f->type == FD_PIPE) {
        // Pipes have no concept of offset
        r = piperead(f->pipe, addr, n);
    } else if (f->type == FD_DEVICE) {
        if(f->major < 0 || f->major >= NDEV || !devsw[f->major].read)
            return -1;
        r = devsw[f->major].read(addr, n);
    } else if (f->type == FD_INODE) { 
        ilock(f->ip);
        // If the file represents an inode,fileread and filewrite use the I/O offset as the offset for the operation and then advance it
        if ((r = readi(f->ip, addr, f->off, n)) > 0)
            f->off += r;
        iunlock(f->ip);
    } else { 
        panic("unsupported file type: %d.\n", f->type);
    }
    return r;
}

/*
 * Write to file f.
 * addr is a user virtual address.
 * It services the system call write
 */
int32_t filewrite(struct file *f, char *addr, int32_t n)
{
    int r = 0;
    int ret = 0;

    if(!f->writable) 
        return -1;

    switch (f->type) {
        case FD_PIPE:
            ret = pipewrite(f->pipe, addr, n);
            break;
        case FD_DEVICE:
            if (f->major < 0 || f->major >= NDEV || !devsw[f->major].write) 
                return -1;
            ret = devsw[f->major].write(addr, n);
            break;
        case FD_INODE: 
        {
            // write a few blocks at a time to avoid exceeding
            // the maximum log transaction size, including
            // i-node, indirect block, allocation blocks,
            // and 2 blocks of slop for non-aligned writes.
            // this really belongs lower down, since writei()
            // might be writing a device like the console.
            int32_t max = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * BSIZE;  // 1536
            int32_t i = 0;
            while (i < n) {
                int n1 = n - i;
                if(n1 > max)
                    n1 = max;
                begin_op();
                ilock(f->ip);
                if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
                    f->off += r;
                iunlock(f->ip);
                end_op();
                if (r != n1)
                    break;
                i += r;
            }
            ret = (i == n) ? n : -1;
            break;
        }
        default:
            panic("unsupported file type %d. \n", f->type);
            break;
    }
    return ret;
}