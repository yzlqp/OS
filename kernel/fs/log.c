/**
 * @file log.c
 * @author ylp
 * @brief Use data logging to resolve crash consistency issues
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "log.h"
#include "../include/stdint.h"
#include "../printf.h"
#include "../lib/string.h"
#include "../proc/proc.h"

/*
 * Log write: Writes the contents of the transaction (TxB, metadata block, and data block) to the log and waits for these writes to complete. 
 * Log Commit: The transaction commit block (including TxE) is written to the log, and once completed, we consider the transaction to have committed successfully. 
 * Add checkpoint: Writes transactions (metadata and data updates) from the log to the final location on disk. 
 * Release: After a period of time, update the log superblock to mark completed checkpointed transactions as idle. 
 */
struct logheader {
    int32_t n;               // the count of log blocks, If n is not 0, n blocks of log need to be written to disk
    int32_t block[LOGSIZE];  // The header block contains an array of sector numbers, one for each of the logged blocks
};

/*
 * A log is just a data structure maintained in memory
 */
struct log {
    struct spinlock lock;
    int32_t start;          // Block number of first log block
    int32_t size;
    int32_t outstanding;    // How many file system calls are currently being executed,The begin_op is incremented by 1, and the end_op is decremented by 1, equal to 0 indicates that no FS SYS calls are currently being executed
    int32_t commiting;      // Is it being submitted, in commit(), please wait.
    int32_t dev;
    struct logheader lh;
};

struct log log;
static void recover_fromlog(void);
static void commit(void);

/*
 * What it does is it reads information about the logging system from the superblock, 
 * including the starting position of the logging area, the size of the logging area, 
 * and so on.  It then calls recover_from_log to start recovery (it can be called with 
 * no negative impact whether there are any logs that need to be recovered). 
 */
void initlog(int dev, struct superblock *sb)
{
    if (sizeof(struct logheader) >= BSIZE)
        panic("initlog: too big logheader.\n");

    init_spin_lock(&log.lock, "log");
    log.start = sb->logstart;
    log.size = sb->nlog;
    log.dev = dev;
    recover_fromlog();   
}

/*
 * Copy committed blocks from log to their home location
 * reads each block from the log and writes it to the proper place in the file system
 */
static void install_trans(int recovering)
{
    for (int32_t tail = 0; tail < log.lh.n; tail++) {
        // read log block
        struct buf *lbuf = bread(log.dev, log.start + tail + 1);
        // read dst
        struct buf *dbuf = bread(log.dev, log.lh.block[tail]);
        // copy block to dst
        memmove(dbuf->data, lbuf->data, BSIZE);
        // write dst to disk
        bwrite(dbuf);  
        // In log_write, bpin the corresponding Cache block. Here we bunpin it, so after this, the Cache block can be recycled by the Buffer Cache 
        if (recovering == 0) {
            bunpin(dbuf);
        }
        brelease(lbuf);
        brelease(dbuf);
    }
}

/*
 * Read logheader from disk into memory 
 */
static void read_head(void)
{
    struct buf *buf = bread(log.dev, log.start);
    struct logheader *lh = (struct logheader *)(buf->data);
    log.lh.n = lh->n;
    for (int32_t i = 0; i < log.lh.n; ++i) {
        log.lh.block[i] = lh->block[i];
    }
    brelease(buf);
}

/*
 * Write logHeader in memory to disk
 * This is the true point at which the
 * current transaction commits.
 */
static void write_head(void)
{
    // Get the logheader cache block from bread
    struct buf *buf = bread(log.dev, log.start);
    struct logheader *hb = (struct logheader *)(buf->data);
    // Update n for logheader on disk
    hb->n = log.lh.n;
    // Update the block number for each log block(data block)
    for (int32_t i = 0; i < log.lh.n; ++i) {
        hb->block[i] = log.lh.block[i];
    }
    // Write the updated logheader back to disk
    // From here, the transaction commit is actually complete, so the crash that started here can be recovered 
    bwrite(buf);
    brelease(buf);
}

/*
 * Read the logheader on the disk. If n=0, the log system does not need to recover. 
 * Therefore, skip recovery and continue to start. If the count n is not equal to 0, 
 * there are logs that need to be replayed, so we go back to the add checkpoint in 
 * the data log tetrad and call Install_trans again to perform the add checkpoint process. 
 * Finally, at the function exit point, the old log is cleared.
 */
static void recover_fromlog(void)
{
    // read logheader
    read_head();
    // if committed, copy from log to disk
    install_trans(1);
    // clear the log
    log.lh.n = 0;
    write_head(); 
}

/*
 * Called at the start of each FS system call.
 */
void begin_op(void)
{ 
    acquire_spin_lock(&log.lock);
    while (1) {
        if (log.commiting) {
            // Wait for the current checkpoint to complete
            sleep(&log, &log.lock);
        } else if (log.lh.n + (log.outstanding + 1) * MAXOPBLOCKS > LOGSIZE) {
            // this op might exhaust log space; wait for commit.
            // until there is enough unreserved log space to hold the writes from this call.
            sleep(&log, &log.lock);
        } else {
            // Now the FS system call is processed
            log.outstanding += 1;
            release_spin_lock(&log.lock);
            break;
        }
    }
}

/*
 * called at the end of each FS system call.
 * commits if this was the last outstanding operation.
 */
void end_op(void)
{
    int do_commit = 0;
    acquire_spin_lock(&log.lock);
    // FS sys calls is ending, decrease log.outstanding
    log.outstanding -= 1;

    if (log.commiting)
        panic("end_op: log.comming = 1");

    if (log.outstanding == 0) {
        // No FS sys calls are currently available, can start the transaction commit
        do_commit = 1;
        log.commiting = 1;
    } else {
        // If there are other FS calls currently in progress, let's not commit yet 
        wakeup(&log);
    }
    release_spin_lock(&log.lock);
    if (do_commit) {
        // call commit w/o holding locks, since not allowed to sleep with locks.
        commit();
        // Change the COMMIT status. Lock protection is required
        acquire_spin_lock(&log.lock);
        // Once you've committed everything, reset to 0 and wake up a suspended FS call, whose begin_op can now resume 
        log.commiting = 0;
        wakeup(&log);
        release_spin_lock(&log.lock);
    }
}

/*
 * Copy modified blocks from cache to log.
 * copies each block modified in the transaction from the buffer cache to its slot in the log on disk.
 */
static void write_log(void)
{
    for (int32_t tail = 0; tail < log.lh.n; tail++) {
        // Read the disk blocks of the log area sequentially from disk, with an extra 1 to skip the logheader 
        struct buf *to = bread(log.dev, log.start + tail + 1);
        // Read the updated Cache block from the Buffer Cache
        struct buf *from = bread(log.dev, log.lh.block[tail]);
        // Copy from from to to With memmove, both cache block copies are now in the updated state 
        memmove(to->data, from->data, BSIZE);
        // Write the updated logged block back to the log area of the disk
        bwrite(to);
        // When both cache blocks are used, call brelse to update the lis
        brelease(from);
        brelease(to);
    }
}

/*
 * Commit a log operation
 */
static void commit(void)
{
    if (log.lh.n > 0) {
        // Write modified blocks from cache to log
        write_log();
        // Write header to disk -- the real commit
        // this is the commit point, and a crash after the write will result in recovery replaying the transaction’s writes from the log
        write_head();
        // Add checkpoint
        install_trans(0);
        // By the time n=0 is reset here, the checkpoint has been successfully completed, but n=0 has not yet been written to disk's logheader 
        log.lh.n = 0;
        // Erase the transaction from the log, The updated n=0 is written to disk's logheader, so the old log is released/reused 
        write_head();
    }
}

/*
 * Caller has modified b->data and is done with the buffer.
 * Record the block number and pin in the cache by increasing refcnt.
 * commit()/write_log() will do the disk write.
 * 
 * When a block is committed multiple times, instead of allocating additional 
 * log space to it, the same slot in the log area continues to be used 
 */
void log_write(struct buf *b)
{
    acquire_spin_lock(&log.lock);
    // Check the number of log blocks that have written the log header. The upper limit is LOGSIZE 
    if (log.lh.n >= LOGSIZE || log.lh.n >= log.size - 1)
        panic("log_write: too big a transaction.\n");
    if (log.outstanding < 1)
        panic("log_write: outside of transaction.\n");
    
    int32_t i;
    for (i = 0; i < log.lh.n; ++i) {
        // If an update to the same block B has already been committed in the log, a new logged block is not allocated 
        if (log.lh.block[i] == b->blockno) {
            break;
        }
    }
    // TODO: A temporary solution
    const uint32_t LBA = 0x20800; 
    log.lh.block[i] = b->blockno - LBA;
    if (i == log.lh.n) {
        // pins the buffer in the block cache to prevent the block cache from evicting it
        bpin(b);
        log.lh.n += 1;
    }
    release_spin_lock(&log.lock);
}