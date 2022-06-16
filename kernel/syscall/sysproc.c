/**
 * @file sysproc.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-03-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "arg.h"
#include "../include/stdint.h"
#include "../proc/proc.h"

extern uint64_t uptime();
extern struct spinlock tickslock;
extern uint64_t ticks;

/*
 * Terminate the current process; status reported to wait(). No return.
 */
int64_t sys_exit()
{
    int64_t n;
    if (argint(0, (uint64_t *)&n) < 0)
        return -1;
    exit(n);
    return 0;
}

/*
 * Return the current process’s PID.
 */
int64_t sys_getpid()
{
    return myproc()->pid;
}

/*
 * Create a process, return child’s PID.
 */
int64_t sys_fork()
{
    return fork();
}

/*
 * Wait for a child to exit; exit status in *status; returns child PID.
 */
int64_t sys_wait()
{ 
    uint64_t *p;
    if (argptr(0, (char **)&p, sizeof(uint64_t)) < 0)
        return -1;
    return wait((int64_t *)p);
}

/*
 * Give up the CPU for one scheduling round.
 */
int64_t sys_yield()
{
    yield();
    return 0;
}

/* 
 * Terminate process PID. Returns 0, or -1 for error
 */
int64_t sys_kill()
{
    int64_t pid;
    if (argint(0, (uint64_t *)&pid) < 0) 
        return -1;
    return kill(pid);
}

/*
 * Grow process’s memory by n bytes. Returns start of new memory.
 */
int64_t sys_sbrk()
{
    int64_t delta;
    if (argint(0, (uint64_t *)&delta) < 0) 
        return -1;
    int64_t oldaddr = myproc()->sz;
    if (growproc(delta) < 0) 
        return -1;
    return oldaddr;
}

/*
 * Gets the value of the current ticks 
 */
int64_t sys_uptime()
{
    return uptime();
}

/* 
 * Pause for n clock ticks.
 */
int64_t sys_sleep()
{
    int64_t n;
    if (argint(0, (uint64_t *)&n) < 0) 
        return -1;
    
    acquire_spin_lock(&tickslock);
    uint64_t ticks0 = ticks;
    while (ticks < ticks0 + n) {
        if(myproc()->killed != 0) {
            release_spin_lock(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release_spin_lock(&tickslock);
    return 0;
}