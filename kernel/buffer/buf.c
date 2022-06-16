/**
 * @file buf.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "buf.h"
#include "sync/spinlock.h"
#include "sync/sleeplock.h"
#include "drivers/mmc/sd.h"
#include "printf.h"

struct bcache bcache;

/*
 * Initialize the system buffer (bcache)
 * First get bcache.lock, then join the NBUF slots into a linked list. 
 * After initialization, all access to the Buffer Cache will go through bcache.
 * head instead of the static array bcache.buf 
 */
void binit()
{   
    init_spin_lock(&bcache.lock, "bcache");

    // Create linked list of buffers
    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;
    for (struct buf *b = bcache.buf; b < bcache.buf + NBUF; ++b) {
        // head interpolation
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        init_sleep_lock(&b->lock, "buffer");
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }
    cprintf("binit: success.\n");
}

/*
 * Look through buffer cache for block on device dev
 * Scans all Cache blocks in the Buffer Cache based on the entered device number and block number. 
 * If the cache hits, bget updates the reference count refCNt, releasing bcache.lock 
 * Bget returns the locked cache block
 */
static struct buf *bget(uint32_t dev, uint32_t blockno)
{
    // When multiple threads access the Buffer Cache, they always wait outside
    acquire_spin_lock(&bcache.lock);

    // Is the block already cached? Head.next points to the most recently used cache block
    for (struct buf *b = bcache.head.next; b != &bcache.head; b = b->next) {
        if (b->dev == dev && b->blockno == blockno) {
            // If hit, return directly
            b->refcnt += 1;
            release_spin_lock(&bcache.lock);
            // Returns the locked cache block
            acquire_sleep_lock(&b->lock);
            return b;
        }
    }

    // If there is no hit, Recycle the least recently used (LRU) unused buffer. 
    // Head.prev points to the least recently used cache block
    for (struct buf *b = bcache.head.prev; b != &bcache.head; b = b->prev) {
        if (b->refcnt == 0 && !(b->flags & BUF_DIRTY)) {
            // refcnt indicates the number of threads currently queuing to read the cache block 
            b->dev = dev;
            b->blockno = blockno;
            b->flags = 0;
            b->refcnt = 1;
            release_spin_lock(&bcache.lock);
            // Returns the locked cache block
            acquire_sleep_lock(&b->lock);
            return b;
        }
    }

    // TODO: If you can't find one that isn't dirty, write back with a dirty one
    panic("bget: no available buffer.\n");
    return NULL;
}

/*
 * Increase the reference count for this BUF
 */
void bpin(struct buf *b)
{
    acquire_spin_lock(&bcache.lock);
    b->refcnt += 1;
    release_spin_lock(&bcache.lock);
}

/*
 * Reduces the reference count for this BUF
 */
void bunpin(struct buf *b)
{
    acquire_spin_lock(&bcache.lock);
    b->refcnt -= 1;
    release_spin_lock(&bcache.lock);
}

/*
 * Write b's contents to disk.  Must be locked
 */
void bwrite(struct buf *b)
{
    if (!is_current_cpu_holing_sleep_lock(&b->lock)) {
        panic("bwrite: buf not locked.\n");
    }
    b -> flags |= BUF_DIRTY;
    // Write disk block
    sd_rw(b);
}

/*
 * Return a locked buf with the contents of the indicated block
 */
struct buf *bread(uint32_t dev, uint32_t blockno)
{
    const uint32_t LBA = 0x20800;   //TODO: Remember to change this area to dynamic parsing, although hard coding is not impossible
    struct buf *b = bget(dev, blockno + LBA);
    if (!(b->flags & BUF_VALID)) {
        // A value valid of 0 indicates that this is a slot that has just been reclaimed
        // Therefore, the disk block must be read from the disk to the slot
        sd_rw(b);
    }
    // The cache block returned is locked
    return b;
}

/*
 * Release a locked buffer
 * Move to the head of the most-recently-used list
 * A kernel thread must release a buffer by calling brelease when it is done with it. 
 * When the caller is done with a buffer, it must call brelse to release it.
 */
void brelease(struct buf *buf)
{
    if (!is_current_cpu_holing_sleep_lock(&buf->lock)) 
        panic("brelease: buf not locked.\n");
    
    // Get the lock on cache block b at bget and release it here
    release_sleep_lock(&buf->lock);

    acquire_spin_lock(&bcache.lock);
    buf->refcnt -= 1;
    if (buf->refcnt == 0) {
        // no one is waiting for it.
        buf->next->prev = buf->prev;
        buf->prev->next = buf->next;
        buf->next = bcache.head.next;
        buf->prev = &bcache.head;
        bcache.head.next->prev = buf;
        bcache.head.next = buf;
    }
    release_spin_lock(&bcache.lock);
}