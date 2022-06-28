/**
 * @file fs.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "fs.h"
#include "log.h"
#include "../file/file.h"
#include "../include/stdint.h"
#include "../include/stat.h"
#include "../buffer/buf.h"
#include "../lib/string.h"
#include "../sync/spinlock.h"
#include "../sync/sleeplock.h"
#include "../include/param.h"
#include "../printf.h"
#include "../proc/proc.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
static struct inode *iget(uint32_t dev, uint32_t inum);

// Each disk has to have a superblock, but we only run one disk, so this unique structure is sufficient
struct superblock sb[1];
struct itable itable;

/*
 * Read the super block
 */
static void readsb(int dev, struct superblock *sb)
{
    struct buf* bp;
    bp = bread(dev, 1);
    memmove(sb, bp->data, sizeof(*sb));
    brelease(bp);
}

/*
 * Initialize the file system
 */
void fsinit(int dev)
{
    readsb(dev, &sb[0]);
    if (sb[0].magic != FSMAGIC) {
        panic("fsinit: invalid file system.\n");
    }
    initlog(dev, &sb[0]);
}

/*
 * zero a block
 */
static void bzero(int dev, int bno)
{
    struct buf* bp = bread(dev, bno);
    memset(bp->data,0,BSIZE);
    log_write(bp);
    brelease(bp);
}

/*
 * Allocate a disk block and return a disk block that has been cleared
 */
static uint32_t balloc(int dev)
{
    for (int b = 0; b < sb[0].size; b += BPB) {
        // Given block number b, return the bitmap block number, Then read the corresponding Bitmap block to the cache
        // Synchronization is implicit in bread, so there is no need to add an additional lock
        struct buf *bp = bread(dev, BBLOCK(b, sb[0]));
        for (int bi = 0; bi < BPB && b + bi < sb[0].size; bi++) {
            // Check the bits in the bitmap block one by one
            int m = 1 << (bi % 8);
            // If the bitmap block corresponding to bi is free
            if ((bp->data[bi / 8] & m) == 0) {
                // Marks the block as used
                bp->data[bi / 8] |= m; 
                log_write(bp);
                brelease(bp);
                // Clear the newly allocated disk block to prevent old content or junk data from being used
                bzero(dev, b + bi);
                // Returns the block number of the newly allocated disk block
                return b + bi;
            }
        }
        brelease(bp);
    }
    panic("balloc: out of blocks.\n");
    return -1;
}

/*
 * Free a disk block.
 * Locate the corresponding bit in the bitmap block and re-mark it as 0, 
 * indicating that the disk block is now free and can be reallocated.
 *  Again, there is no need to explicitly add locks.
 */
static void bfree(int dev, uint32_t b)
{
    // Given block number b, return the bitmap block number
    struct buf *bp = bread(dev, BBLOCK(b, sb[0]));
    int bi = b % BPB;
    int m = 1 << (bi % 8);
    if ((bp->data[bi / 8] & m) == 0)
        panic("bfree: freeing free block.\n");
    bp->data[bi / 8] &= ~m;
    log_write(bp);
    brelease(bp);
}

/*
 * initialize the inode table
 */
void iinit()
{
    init_spin_lock(&itable.lock, "itable");
    for (int i = 0; i < NINODE; i++) {
        init_sleep_lock(&itable.inode[i].lock, "inode");
    }
}

/*
 * Allocates an inode on disk, Mark it as allocated by giving it type type, Returns an unlocked but allocated and referenced inode.
 */
struct inode *ialloc(uint32_t dev, uint16_t type)
{
    for (int inum = 1; inum < sb->ninodes; ++inum) {
        // Implicit synchronization in bread's bget 
        struct buf *bp = bread(dev, IBLOCK(inum, sb[0]));
        struct dinode *dip = (struct dinode *)bp->data + inum % IPB;
        if (dip->type == 0) {
            // this is a free inode
            memset(dip, 0, sizeof(*dip));
            dip->type = type;
            // Cache block bp is modified, write to log, mark it allocated on the disk
            log_write(bp);
            brelease(bp);
            // Call iget to cache the inode in itable
            return iget(dev, inum);
        }
        brelease(bp); 
    }
    panic("balloc: no available inodes.\n");
    return NULL;
}

/*
 * Copy a modified in-memory inode to disk.
 * ust be called after every change to an ip->xxx field
 * that lives on disk, since i-node cache is write-through.
 * Caller must hold ip->lock.The inode cache is write-through, 
 * which means that code that modifies a cached inode must 
 * immediately write it to disk with iupdate.
 */
void iupdate(struct inode *ip)
{
    struct buf *bp = bread(ip->dev, IBLOCK(ip->inum, sb[0]));
    struct dinode *dip = (struct dinode *)bp->data + ip->inum % IPB;
    // ip is the new inode cache copy, dip is the old disk inode
    dip->type = ip->type;
    dip->major = ip->major;
    dip->minor = ip->minor;
    dip->nlink = ip->nlink;
    dip->size = ip->size;
    memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
    // This update is written to log
    log_write(bp);
    brelease(bp);
}

/*
 * Find the inode with number inum on device dev
 * and return the in-memory copy. Does not lock
 * the inode and does not read it from disk.
 * iget() provides non-exclusive access to an inode, 
 * so that there can be many pointers to the same inode
 */
static struct inode * iget(uint32_t dev, uint32_t inum)
{
    acquire_spin_lock(&itable.lock);
    struct inode *empty = NULL;
    struct inode *ip = NULL;
    // Is the inode already cached?
    for (ip = &itable.inode[0]; ip < &itable.inode[NINODE]; ++ip) {
        // Check the icache cache to see if it is already cached
        if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
            ip->ref++;
            release_spin_lock(&itable.lock);
            return ip;
        }
        // Find the first empty icache slot
        if (empty == NULL && ip->ref == 0)
            empty = ip;
    }
    // Recycle an inode cache entry.
    if (empty == NULL)
        panic("iget: no available inodes.\n");
    // Empty is the first empty slot in the icache 
    ip = empty;
    ip->dev = dev;
    ip->inum = inum;
    ip->ref = 1;
    ip->valid = 0;
    release_spin_lock(&itable.lock);
    return ip;
}

/*
 * Increase the reference count of the specified inode
 */
struct inode *idup(struct inode *ip)
{
    acquire_spin_lock(&itable.lock);
    ip->ref++;
    release_spin_lock(&itable.lock);
    return ip;
}

/*
 * Lock the given inode.
 * Reads the inode from disk if necessary.
 * The struct inode that iget returns may not have any useful content. 
 * In order to ensure it holds a copy of the on-disk inode, code must call ilock. 
 * This locks the inode (so that no other process can ilock it) and reads the inode from the disk, if it has not already been read.
 * Multiple processes can hold a C pointer to an inode returned by iget, but only one process can lock the inode at a time.
 */
void ilock(struct inode *ip)
{
    struct buf *bp;
    struct dinode *dip;

    if (ip == NULL || ip->ref < 1)
        panic("ilock");

    acquire_sleep_lock(&ip->lock);

    // The inode pointer just obtained from iget is valid=0
    if (ip->valid == 0) {
        bp = bread(ip->dev, IBLOCK(ip->inum, sb[0]));
        dip = (struct dinode *)bp->data + ip->inum % IPB;
        ip->type = dip->type;
        ip->major = dip->major;
        ip->minor = dip->minor;
        ip->nlink = dip->nlink;
        ip->size = dip->size;
        memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
        brelease(bp);
        ip->valid = 1;
        if(ip->type == 0)
            panic("ilock: no type");
    }
}

/*
 * Unlock the given inode.
 */
void iunlock(struct inode *ip)
{
    if (ip == NULL || !is_current_cpu_holing_sleep_lock(&ip->lock) || ip->ref < 1)
        panic("ilock.\n");

    release_sleep_lock(&ip->lock);
}

/*
 * Truncate inode (discard contents).
 * Caller must hold ip->lock.
 * truncate the file to zero bytes, freeing the data blocks 
 * sets the inode type to 0 ，and writes the inode to disk
 */
void itrunc(struct inode *ip)
{
    // Free all direct blocks
    for (int i = 0; i < NDIRECT; ++i) {
        if (ip->addrs[i] != 0) {
            bfree(ip->dev, ip->addrs[i]);
            ip->addrs[i] = 0;
        }  
    }
    // Release all indirect blocks
    if (ip->addrs[NDIRECT] != 0) {
        struct buf *bp = bread(ip->dev, ip->addrs[NDIRECT]);
        uint32_t *a = (uint32_t *)bp->data;
        for (int j = 0; j < NINDIRECT; ++j) {
            if (a[j] != 0)
                bfree(ip->dev, a[j]);
        }
        brelease(bp);
        // Release the indirect block
        bfree(ip->dev, ip->addrs[NDIRECT]);
        ip->addrs[NDIRECT] = 0;
    }
    ip->size = 0;
    // The inode is changed, so it is updated to disk
    iupdate(ip);
}

/*
 * Drop a reference to an in-memory inode.
 * If that was the last reference, the inode cache entry can be recycled.
 * If that was the last reference and the inode has no links to it, free the inode (and its content) on disk.
 */
void iput(struct inode *ip)
{
    acquire_spin_lock(&itable.lock);
    if (ip->ref == 1 && ip->valid && ip->nlink == 0) {
        // The inode on disk is released only if ref = 0 and nlink = 0, no pointer or directory entry refers to the inode
        acquire_sleep_lock(&ip->lock);
        release_spin_lock(&itable.lock);

        itrunc(ip);
        ip->type = 0;
        iupdate(ip);
        ip->valid = 0;

        release_sleep_lock(&ip->lock);
        acquire_spin_lock(&itable.lock);
    }
    ip->ref--;
    release_spin_lock(&itable.lock);
}

/*
 * Unlock the given inode.
 * and then drop a reference to an in-memory inode.
 */
void iunlockandput(struct inode *ip) 
{
    iunlock(ip);
    iput(ip);
}

/*
 * Returns the disk block number of block bn data block in the specified inode
 * If you don't have this block then allocate one
 */
static uint32_t bmap(struct inode *ip, uint32_t bn)
{
    // Find the direct block first
    if (bn < NDIRECT) {
        uint32_t addr = ip->addrs[bn];
        // The direct block has not been allocated yet, so a new block is allocated
        if (addr == 0)
            ip->addrs[bn] = addr = balloc(ip->dev);
        return addr;
    }
    bn -= NDIRECT;
    // If bn is not a direct block, look for an indirect block
    if (bn < NINDIRECT) {
        // Load indirect block, allocating if necessary.
        uint32_t addr = ip->addrs[NDIRECT];
        // If the first level indirect mapping block is empty, allocate a first level indirect mapping block
        if (addr == 0) 
            ip->addrs[NDIRECT] = addr = balloc(ip->dev);
        // Read the indirection block
        struct buf *bp = bread(ip->dev, addr);
        uint32_t *a = (uint32_t *)bp->data;
        if ((addr = a[bn]) == 0) {
            a[bn] = addr = balloc(ip->dev);
            // Balloc will only write the new bitmap to the log, and the first-level indirect block is now updated, 
            // so a separate call to log_write will write the update to the log 
            log_write(bp);
        }
        brelease(bp);
        return addr;
    }
    panic("bmap bn: %d is out of range.\n", bn);
    return -1;
}

/*
 * Copy stat information from inode.
 * Caller must hold ip->lock.
 */
void stati(struct inode *ip, struct stat *st)
{
    st->dev = ip->dev;
    st->ino = ip->inum;
    st->type = ip->type;
    st->nlink = ip->nlink;
    st->size = ip->size;
}

/*
 * Read data from inode. Caller must hold ip->lock.
 */
int readi(struct inode *ip, char *dst, uint32_t offset, uint32_t n)
{
    if (offset > ip->size || offset + n < offset)
        return 0;
    // Reads are legal, but the number of bytes cannot exceed the file size
    if (offset + n > ip->size)
        n = ip->size - offset;

    uint32_t m, tot;
    for (tot = 0; tot < n; tot += m, offset += m, dst += m) {
        struct buf *b = bread(ip->dev, bmap(ip, offset / BSIZE));
        m = min(n - tot, BSIZE - offset % BSIZE);
        memmove(dst, b->data + (offset % BSIZE), m);
        brelease(b);
    }
    // Returns the size of the read
    return tot;
}

/*
 * Write data to inode. Caller must hold ip->lock.
 * If the file grows, update its inode size information
 */
int writei(struct inode *ip, char *src, uint32_t offset, uint32_t n)
{
    if (offset > ip->size || offset + n < offset)
        return -1;
    if(offset + n > MAXFILE * BSIZE) 
        return -1;

    // copies data into the buffers 
    uint32_t tot = 0, m = 0;
    for (tot = 0; tot < n; tot += m, offset += m, src += m) {
        struct buf *bp = bread(ip->dev, bmap(ip, offset / BSIZE));
        m = min(n - tot, BSIZE - offset / BSIZE);
        memmove(bp->data + offset % BSIZE, src, m);
        // The cache block is modified, and the update is written to the log
        log_write(bp);
        brelease(bp);
    }
    if (offset > ip->size) 
        ip->size = offset;
    // write the i-node back to disk even if the size didn't change
    // because the loop above might have called bmap() and added a new
    // block to ip->addrs[].
    iupdate(ip);
    return tot;
}

/*
 * Name comparison function
 */
int namecmp(const char *s, const char *t)
{
    return strncmp(s, t, DIRSIZ);
}

/*
 * Look for a directory entry in a directory.
 * If found, set *poff to byte offset of entry.
 */
struct inode *dirlookup(struct inode *dp, char *name, uint32_t *poff)
{
    if (dp->type != T_DIR)
        panic("dirlookup: not a DIR");

    for (uint32_t off = 0; off < dp->size; off += sizeof(struct dirent)) {
        struct dirent de;
        if (readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
            panic("dirlookup: readi failed.\n");
        if(de.inum == 0) 
            continue;
        if (namecmp(name, de.name) == 0) {
            // entry matches path element
            if (poff) 
                *poff = off;
            return iget(dp->dev, de.inum);
        }
    }
    return NULL;
}

/*
 * Write a new directory entry (name, inum) into the directory dp.
 */
int dirlink(struct inode *dp, char *name, uint32_t inum)
{
    int off;
    struct dirent de;
    struct inode *ip;
    // Check that name is not present.
    if ((ip = dirlookup(dp, name, NULL)) != NULL) {
        // the name already exists
        iput(ip);
        return -1;
    }
    // Look for an empty dirent.
    for (off = 0; off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de)) 
            panic("dirlink: read error.\n");
        if(de.inum == 0)
            break;
    }
    strncpy(de.name, name, DIRSIZ);
    de.inum = inum;
    // adds a new entry to the directory by writing at offset off
    if (writei(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
        panic("dirlink: write error.\n");
    return 0;
}

/*
 * Copy the next path element from path into name.
 * Return a pointer to the element following the copied one.
 * The returned path has no leading slashes,
 * so the caller can check *path=='\0' to see if the name is the last one
 * If no name to remove, return 0.
 * Examples:
 * skipelem("a/bb/c", name) = "bb/c", setting name = "a"
 * skipelem("///a//bb", name) = "bb", setting name = "a"
 * skipelem("a", name) = "", setting name = "a"
 * skipelem("", name) = skipelem("////", name) = 0
 */
static char* skipelem(char *path, char *name)
{
    char *s;
    int len;    
    while (*path == '/')
        path++;
    if (*path == 0)
        return 0;
    s = path;
    while (*path != '/' && *path != 0)
        path++;
    len = path - s;
    if (len >= DIRSIZ) {
        memmove(name, s, DIRSIZ);
    } else {
        memmove(name, s, len);
        name[len] = 0;
    }
    while (*path == '/')
        path++;
    return path;
}

/*
 * Look up and return the inode for a path name.
 * If parent != 0, return the inode for the parent and copy the final
 * path element into name, which must have room for DIRSIZ bytes.
 * Must be called inside a transaction since it calls iput().
 * if an invocation of namex by one kernel thread is blocked on a disk I/O, 
 * another kernel thread looking up a different pathname can proceed concurrently.
 */
static struct inode *namex(char *path, int nameiparent, char *name)
{
    // Decide where to start, starting with the root directory if there is a '/', or the current directory otherwise 
    struct inode *ip = (*path == '/') ? iget(ROOTDEV, ROOTINO) : idup(myproc()->cwd);
    // uses skipelem to consider each element of the path in turn
    while ((path = skipelem(path, name)) != 0) {
        ilock(ip);
        // Check whether the ip address of the current node is a directory
        if (ip->type != T_DIR) {
            iunlock(ip);
            return NULL;
        }
        if (nameiparent && *path == '\0') {
            // path='\0' indicates that name is the last element
            iunlock(ip);
            return ip;
        }
        struct inode *next = dirlookup(ip, name, 0);
        if (next == NULL) {
            // Cannot be found under current directory node
            iunlock(ip);
            return NULL;
        }
        iunlock(ip);
        ip = next;
    }
    if (nameiparent) {
        // Normally nameiParent should return in the main loop
        iput(ip);
        return 0;
    }
    // When the loop runs out of path elements, it returns
    return ip;
}

/*
 * Returns the inode of the parent directory of the last element
 */
struct inode *nameiparent(char *path, char *name)
{
    // Where name is not a temporary buffer, the caller provides name[] to load it because the caller still needs it 
    return namex(path, 1, name);
}

/*
 * Returns the inode of the last element in the pathname
 */
struct inode *namei(char *path)
{
    char name[DIRSIZ];
    //evaluates path and returns the corresponding inode
    return namex(path, 0, name);
}