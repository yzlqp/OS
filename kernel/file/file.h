/**
 * @file file.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef FILE_H
#define FILE_H

#include "../fs/fs.h"
#include "../sync/sleeplock.h"

/*
 * File structure, some of the following fields may be able to use union, implement first and then worry about optimization
 * Each call to open creates a new open file (a new struct file)
 */
struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type;
    int ref;                // reference count
    char readable;          // Whether the file can be read
    char writable;          // Whether the file can be writtens
    struct pipe *pipe;      // FD_PIPE
    struct inode *ip;       // FD_INODE and FD_DEVICE
    size_t off;             // File offset for FD_INODE  
    int16_t major;          // FD_DEVICE
};

#define major(dev)  ((dev)>>16 & 0xFFFF)
#define minor(dev)  ((dev) & 0xFFFF)
#define mkdev(m, n) ((uint32_t)((m) << 16) | (n))

// The corresponding read/write function is mapped by the master device number
struct devsw {
    int (*read)(char *, int);
    int (*write)(char *, int);
};

extern struct devsw devsw[];
#define CONSOLE 1

/**
 * @brief  Initialize the kernel file table
 * @retval None
 */
void file_init(void);

/**
 * @brief  Allocate a file structure.
 * @retval Point to a file structure.
 */
struct file *filealloc(void);

/**
 * @brief  Create a copy of the open file struct file, just Increment ref count for file f.
 * @param  file*: It then returns a pointer to the same struct file
 * @retval A pointer to file structure
 */
struct file *filedup(struct file *f);

/**
 * @brief  Close specified files (Decrement ref count, close when reaches 0.)
 * @param  file*: A pointer to file structure
 * @retval None
 */
void fileclose(struct file *f);

/**
 * @brief  Get metadata about file f.
 * @param  file*: A pointer to file structure
 * @param  stat*: A pointer to stat structure
 * @retval 0 means success and -1 means failure
 */
int32_t filestat(struct file *, struct stat *);

/**
 * @brief  Read from file f.
 * @param  *f: A pointer to file structure
 * @param  *addr: addr is a user virtual address.
 * @param  n: Number of bytes read
 * @retval Number of bytes read, -1 is error
 */
int32_t fileread(struct file *f, char *addr, int32_t n);

/**
 * @brief  Write to file f. It services the system call write
 * @param  f: A pointer to file structure
 * @param  *addr: addr is a user virtual address.
 * @param  n: Number of bytes written
 * @retval Number of bytes written, -1 is error
 */
int32_t filewrite(struct file *f, char *addr, int32_t n);

#endif /* FILE_H */