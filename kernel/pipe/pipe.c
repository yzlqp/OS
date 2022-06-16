/**
 * @file pipe.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-05-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../file/file.h"
#include "../proc/proc.h"
#include "pipe.h"

/*
 * Apply for and assign a PIPE 0 means success and -1 means failure
 */
int pipealloc(struct file **f0, struct file **f1)
{
    struct pipe *pi = NULL;
    *f0 = NULL;
    *f1 = NULL;
    if ((*f0 = filealloc()) == NULL || (*f1 = filealloc()) == NULL) 
        goto bad;
    if ((pi = (struct pipe *)kalloc(sizeof(struct pipe))) == NULL) 
        goto bad;

    pi->is_readopen = 1;
    pi->is_writeopen = 1;
    pi->nread = 0;
    pi->nwrite = 0;
    init_spin_lock(&pi->lock, "pipe");
    (*f0)->type = FD_PIPE;
    (*f0)->readable = 1;
    (*f0)->writable = 0;
    (*f0)->pipe = pi;

    (*f1)->type = FD_PIPE;
    (*f1)->readable = 0;
    (*f1)->writable = 1;
    (*f1)->pipe = pi;

    return 0;

bad:
    if (pi != NULL)
        kfree(pi);
    if (*f0 != NULL)
        fileclose(*f0);
    if (*f1 != NULL)
        fileclose(*f1);
    
    return -1;
}

/* 
 * Close a pipe
 */
void pipeclose(struct pipe *pi, int writable)
{
    acquire_spin_lock(&pi->lock);
    if (writable) {
        pi->is_writeopen = 0;
        wakeup(&pi->nread);
    } else {
        pi->is_readopen = 0;
        wakeup(&pi->nwrite);
    }
    if (pi->is_readopen == 0 && pi->is_writeopen == 0) {
        release_spin_lock(&pi->lock);
        kfree(pi);
    } else {
        release_spin_lock(&pi->lock);
    }
}

/* 
 * We start by acquiring the lock of the pipe, then write data to the pipe in a for loop. 
 * After writing enough n bytes, we wake up the PipeREAD process on the Nread channel, 
 * and then jump out of the for loop, release the lock, and return successfully. If the buffer 
 * is full while writing a byte, it falls into a while loop, wakes up the PipeREAD process on the 
 * Nread channel, and suspends itself to sleep on the Nwrite channel.
 */
int32_t pipewrite(struct pipe *pi, void *addr, int32_t n)
{
    int i = 0;
    struct proc *p = myproc();
    acquire_spin_lock(&pi->lock);

    while (i < n) {
        // Pipes that no one reads, don't write
        if (pi->is_readopen == 0 || p->killed != 0) {
            release_spin_lock(&pi->lock);
            return -1;
        }
        // Wait if the pipe write full
        if (pi->nwrite == pi->nread + PIPE_SIZE) {
            wakeup(&pi->nread);
            sleep(&pi->nwrite, &pi->lock);
        } else {
            pi->data[pi->nwrite++ % PIPE_SIZE] = *((char*)addr + i++);
        }
    }
    wakeup(&pi->nread);
    release_spin_lock(&pi->lock);
    return i;
}

/*
 * First get the lock of the pipe, check whether the buffer is empty,
 * if so, directly hang yourself to sleep on the Nread channel; Otherwise,
 * we can read all the bytes from the buffer. =n), finally wake up the Pipewrite 
 * process on the Nwrite channel, release the lock, and return.
 */
int32_t piperead(struct pipe *pi, void *addr, int32_t n)
{
    int i;
    struct proc *p = myproc();
    acquire_spin_lock(&pi->lock);

    // If the pipe is available but empty
    while (pi->nread == pi->nwrite && pi->is_writeopen) {
        if (p->killed != 0) {
            release_spin_lock(&pi->lock);
            return -1;
        }
        // pipe read sleep
        sleep(&pi->nread, &pi->lock);
    }
    // pipe read copy
    for (i = 0; i < n; ++i) {
        if(pi->nread == pi->nwrite) 
            break;
        *((char*)addr + i) = pi->data[pi->nread++ % PIPE_SIZE];
    }
    // pipe write wakeup
    wakeup(&pi->nwrite);
    release_spin_lock(&pi->lock);
    return i;
}