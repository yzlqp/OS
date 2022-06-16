/**
 * @file pipe.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-05-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef PIPE_H
#define PIPE_H

#include "../include/param.h"
#include "../printf.h"
#include "../sync/spinlock.h"
#include "../sync/sleeplock.h"
#include "../file/file.h" 
#include "../memory/kalloc.h"
#include "../include/stdint.h"

#define PIPE_SIZE   (512)

/*
 * The buffer is cyclic, that is, data[0] is next to data[pipesize-1], but the count 
 * is not cyclic, so we need to use the modulo operation 
 */
struct pipe {
    struct spinlock lock;
    char data[PIPE_SIZE];
    uint32_t nread;             // number of bytes read
    uint32_t nwrite;            // number of bytes written
    int32_t is_readopen;        // read fd is still open
    int32_t is_writeopen;       // write fd is still open
};

/**
 * @brief Apply for and assign a PIPE
 * 
 * @param f0 Point to the address of file reads pipe
 * @param f1 Point to the address of file write pipe
 * @return int 0 means success and -1 means failure
 */
int pipealloc(struct file **f0, struct file **f1);

/**
 * @brief Close a pipe
 * 
 * @param pi The address of the pipe descriptor to close
 * @param writable Whether it is writable
 */
void pipeclose(struct pipe *pi, int writable);

/**
 * @brief Write pipe function
 * 
 * @param pi The address of the pipe descriptor
 * @param addr Points to the buffer header address
 * @param n Number of bytes to write
 * @return int32_t Returns the number of bytes write, -1 indicating failure
 */
int32_t pipewrite(struct pipe *pi, void* addr, int32_t n);

/**
 * @brief Read pipe function
 * 
 * @param pi The address of the pipe descriptor
 * @param addr Points to the buffer header address
 * @param n Number of bytes to read
 * @return int32_t Returns the number of bytes read, -1 indicating failure
 */
int32_t piperead(struct pipe *pi, void* addr, int32_t n);

#endif /* PIPE_H */