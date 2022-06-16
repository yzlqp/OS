/**
 * @file buf.h
 * @author ylp
 * @brief The Buffer Cache uses the LRU algorithm to reclaim these slots.
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef BUF_H
#define BUF_H

#include "fs/fs.h"
#include "include/stdint.h"
#include "sync/sleeplock.h"

#define BUF_VALID   0x1     // 0b01  indicates that the buffer contains a copy of the block or not
#define BUF_DIRTY   0x2     // 0b10

struct buf {
    int flags;              // Holds the valid and dirty flag bits
    uint32_t dev;           // Device ID
    uint32_t blockno;       // block number, but more exactly, is the sector number
    uint8_t data[BSIZE];    // Stored data
    uint32_t refcnt;        // How many kernel threads are currently queuing to read this cache block
    struct sleeplock lock;  // The sleep lock of each cache block protects reads and writes to that block
    struct buf *prev;       // LRU cache list
    struct buf *next;       // LRU cache list
};

struct bcache {
    struct spinlock lock;   // The Buffer Cache's spin locks protect information about which blocks have been cached 
    struct buf buf[NBUF];       
    // Linked list of all buffers, through prev/next.
    // Sorted by how recently the buffer was used.
    // head.next is most recent, head.prev is least.
    // i.e. LRU replace algorithm
    struct buf head;
};

/**
 * @brief  Initialize the system buffer
 * @retval None
 */
void binit();

/**
 * @brief  Return a locked buf with the contents of the indicated block
 * @param  dev: The device to read
 * @param  blockno: block number
 * @retval struct buf * 
 */
struct buf *bread(uint32_t dev, uint32_t blockno);

/**
 * @brief  Write the corresponding BUF to the device, at which point the BUF must be locked
 * @param  buf *: Pointer to the buffer to operate on
 * @retval None
 */
void bwrite(struct buf *);

/**
 * @brief  Release a locked buffer and move it to the LRU list
 * @param  buf *: Pointer to the buffer to operate on
 * @retval None
 */
void brelease(struct buf *);

/**
 * @brief  Increase the reference count for this BUF
 * @param  buf *: Pointer to the buffer to operate on
 * @retval None
 */
void bpin(struct buf *);

/**
 * @brief  Reduces the reference count for this BUF
 * @param  buf *: Pointer to the buffer to operate on
 * @retval None
 */
void bunpin(struct buf *);

#endif /* BUF_H */