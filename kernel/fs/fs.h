/**
 * @file fs.h
 * @author ylp
 * @brief Structure of the file system:
 *        [ boot block | Super block | log | inode blocks | Bit map | data blocks]
 *        0            1             2    32              45       46           1000
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef FS_H
#define FS_H

#include "../include/stdint.h"
#include "../include/param.h"
#include "../sync/sleeplock.h"
#include "../include/stat.h"

#define ROOTINO       1                                     // root inode number
#define BSIZE         512                                   // block size
#define FSMAGIC       0x10203040                            // Magic number used to identify the file system
#define NDIRECT       12                                    // Twelve directly addressing blocks
#define NINDIRECT     (BSIZE / sizeof(uint32_t))
#define MAXFILE       (NDIRECT + NINDIRECT)                 // Maximum number of blocks allowed for a file
#define IPB           (BSIZE / sizeof(struct dinode))       // How many inodes can each block hold on a disk
#define IBLOCK(i, sb) (((i) / IPB) + sb.inodestart)         // Given an inode, get the offset on the disk
#define DIRSIZ        14                                    // Directory name Length
#define BPB           (BSIZE * 8)                           // Bitmap bits per block.
#define BBLOCK(b, sb) (((b) / IPB) + sb.bmapstart)

/*
 * Superblock Structure
 * Metadata contains complete file system metadata, such as file system size 
 * or number of blocks, number of data blocks, number of inodes, and number of log blocks 
 * The superblock houses a separate program called MKFS (make File System, located at MKFS /mkfs.c) 
 * that builds and mounts the original file system 
 */
struct superblock {
    uint32_t magic;       // Magic number used to identify the file system
    uint32_t size;        // Total size in bits
    uint32_t nblocks;     // The number of total blocks
    uint32_t ninodes;     // The number of total inode
    uint32_t nlog;        // The number of total log blocks
    uint32_t logstart;    // The start of the log block
    uint32_t inodestart;  // The start of the inode block
    uint32_t bmapstart;   // The start of the bitmap block
};

/*
 * On-disk inode structure
 */
struct dinode {
    uint16_t type;                  // File type, files, directories, or special files (devices), 0 indicates dinode is free  
    uint16_t major;                 // Major device number (T_DEVICE only)
    uint16_t minor;                 // Minor device number (T_DEVICE only)
    uint16_t nlink;                 // (Number of hard links) Number of links to inode in file system, won’t free an inode if its link count is greater than 0 
    uint32_t size;                  // The number of bytes of content in the file
    uint32_t addrs[NDIRECT + 1];    // Data block addresses, 12 direct blocks, and the 13th is a first order indirect block
};

/*
 * in-memory copy of an inode(struct dinode)
 * kernel stores an inode in memory only if there are C pointers referring to that inode
 */
struct inode {
    uint32_t dev;           // device number
    uint32_t inum;          // inode number
    int ref;                // reference count, The number of Pointers in memory to this inode
    struct sleeplock lock;  // Protects everything below here
    int valid;              // inode has been read from disk?
    // copy of disk inode
    uint16_t type;
    uint16_t major;
    uint16_t minor;
    uint16_t nlink;
    uint32_t size;
    uint32_t addrs[NDIRECT + 1];
};

/*
 * The cache used to cache inodes 
 */
struct itable {
    struct spinlock lock;       // The spin lock is used to protect the icache cache
    struct inode inode[NINODE]; // Each inode's sleep lock is used to protect each inode's information
};

/*
 * A directory 's inode has type T_DIR and its data is a sequence of directory entries.
 * Directory entries with inode number zero are free.
 */
struct dirent {
    uint16_t inum;
    char name[DIRSIZ];
};

/**
 * @brief  Drop a reference to an in-memory inode.
 * @param  *ip: Pointer to an in-memory inode.
 * @retval None
 */
void iput(struct inode *ip);

/**
 * @brief  Lock the given inode.
 * @param  *ip: Pointer to an in-memory inode.
 * @retval None
 */
void ilock(struct inode *ip);

/**
 * @brief  Unlock the given inode.
 * @param  *ip: Pointer to an in-memory inode.
 * @retval None
 */
void iunlock(struct inode *ip);

/**
 * @brief   Allocates an inode on disk, Mark it as allocated by giving it type type
 * @param  dev: Devvice number
 * @param  type: File type, files, directories, or special files (devices), 0 indicates dinode is free
 * @retval  Returns an unlocked but allocated and referenced inode.
 */
struct inode *ialloc(uint32_t dev, uint16_t type);

/**
 * @brief Truncate inode (discard contents), Caller must hold ip->lock.
 * @param  *ip: Pointer to an in-memory inode.
 * @retval None
 */
void itrunc(struct inode *ip);

/**
 * @brief  Copy stat information from inode, Caller must hold ip->lock.
 * @param  *ip: Pointer to an in-memory inode.
 * @param  *st: Pointer to a stat structure
 * @retval None
 */
void stati(struct inode *ip, struct stat *st);

/**
 * @brief  Read data from inode. Caller must hold ip->lock.
 * @param  *ip: Pointer to an in-memory inode.
 * @param  *dst: Data buffer address
 * @param  offset: Start at offset 
 * @param  n: Number of bytes to read
 * @retval Number of bytes that have been read. -1 indicates failure
 */
int readi(struct inode *ip, char *dst, uint32_t offset, uint32_t n);

/**
 * @brief  Write data to inode. Caller must hold ip->lock, If the file grows, update its inode size information
 * @param  *ip: Pointer to an in-memory inode.
 * @param  *src: Data buffer address
 * @param  offset: Start at offset 
 * @param  n: Number of bytes to write
 * @retval Number of bytes that have been written. -1 indicates failure
 */
int writei(struct inode *ip, char *src, uint32_t offset, uint32_t n);

/**
 * @brief  Initialize the file system
 * @retval None
 */
void fsinit(int);

/**
 * @brief  initialize the inode table
 * @retval None
 */
void iinit(void);

/**
 * @brief  Returns the inode of the parent directory of the last element
 * @param  *path: The directory to look up
 * @param  *name: The name to look up
 * @retval Returns the inode of the parent directory of the last element
 */
struct inode *nameiparent(char *path, char *name);

/**
 * @brief  Unlock the given inode, and then drop a reference to an in-memory inode.
 * @param  *ip: Pointer to an in-memory inode.
 * @retval None
 */
void iunlockandput(struct inode *ip);

/**
 * @brief  Look for a directory entry in a directory. 
 * @param  *dp: a directory inode pointer
 * @param  *name: The name to look up
 * @param  *poff: If found, set *poff to byte offset of entry.
 * @retval Inode pointer has found or NULL
 */
struct inode *dirlookup(struct inode *dp, char *name, uint32_t *poff);

/**
 * @brief  Copy a modified in-memory inode to disk.
 * @param  *ip: The inode to update 
 * @retval None
 */
void iupdate(struct inode *ip);

/**
 * @brief  Write a new directory entry (name, inum) into the directory dp.
 * @param  *dp: directory inode pointer
 * @param  *name: file name
 * @param  inum: file inode
 * @retval 0 means success and -1 means failure
 */
int dirlink(struct inode *dp, char *name, uint32_t inum);

/**
 * @brief  Returns the inode of the last element in the pathname
 * @param  *path: path name 
 * @retval Returns the inode of the last element in the pathname
 */
struct inode *namei(char *path);

/**
 * @brief  Name comparison function
 * @param  *s: The first name to compare
 * @param  *t: The second name to compare
 * @retval 0 means equal, > 0 or < 0 means unequal
 */
int namecmp(const char *s, const char *t);

/**
 * @brief  Increase the reference count of the specified inode
 * @param  *ip: Pointer to an in-memory inode.
 * @retval Pointer to an in-memory inode.
 */
struct inode *idup(struct inode *ip);

#endif /* FS_H */