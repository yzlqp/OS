/**
 * @file syscall.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "include/stdint.h"
#include "syscall.h"
#include "arch/aarch64/include/trapframe.h"
#include "proc/proc.h"
#include "sysproc.h"
#include "../printf.h"

/*
 * The system call table, this is an array of Pointers, each element pointing to a function
 */
static int64_t (*syscalls[])(void) = {
    [SYS_exec] sys_exec,
    [SYS_exit] sys_exit,
    [SYS_getpid] sys_getpid,
    [SYS_fork] sys_fork,
    [SYS_wait] sys_wait,
    [SYS_pipe] sys_pipe,
    [SYS_yield] sys_yield,
    [SYS_chdir] sys_chdir,
    [SYS_kill] sys_kill,
    [SYS_sbrk] sys_sbrk,
    [SYS_uptime] sys_uptime,
    [SYS_sleep] sys_sleep,
    [SYS_fstat] sys_fstat,
    [SYS_mknod] sys_mknod,
    [SYS_mkdir] sys_mkdir,
    [SYS_open] sys_open,
    [SYS_close] sys_close,
    [SYS_read] sys_read,
    [SYS_write] sys_write,
    [SYS_dup] sys_dup,
    [SYS_link] sys_link,
    [SYS_unlink] sys_unlink
};

/*
 * Generic system call functions
 */
int64_t syscall(struct trapframe *frame)
{
    int64_t call_number = (int64_t)frame->regs[8];
    if (call_number > 0 && call_number < ARRAY_SIZE(syscalls)) {
        int64_t (*fun)(void) = syscalls[call_number];
        if (fun == NULL) {
            goto unsupported_syscall;
        }
        return fun();
    }

unsupported_syscall:
    panic("syscall: unsupported syscall number %d\n", call_number);
    return -1;
}

/* 
 * Get int64 value at user's virtual address ADDR, assign it to * IP and return -1 on failure
 */
int64_t fetchint64ataddr(uint64_t addr, uint64_t *ip)
{
    struct proc *proc = myproc();
    if (addr >= proc->sz || addr + sizeof(*ip) > proc->sz)
        return -1;
    *ip = *(int64_t *)(addr);
    return 0;
}

/* 
 * Fetch the nul-terminated string at addr from the current process.
 * Returns length of string, not including nul, or -1 for error.
 * The kernel implements functions that safely transfer data to and from user-supplied addresses.
 * Points *p to the given string ending in '\0' in the user's virtual address.
 * Returns the length of the string on success, and -1 on failure
 */
int64_t fetchstr(uint64_t addr, char **p)
{
    struct proc *proc = myproc();
    if(addr >= proc->sz)
        return -1;
    char *ep = (char *)proc->sz;
    for (char *s = (char *)addr; s < ep; ++s) {
        if (*s == '\0') {
            *p = (char *)addr;
            return ((int64_t)s - (int64_t)addr);
        }
    }
    return -1;
}

/*
 * Get the nth parameter in the system call. Up to 6 parameters are supported, that is, 0-5 parameters
 * Returns 0 on success, -1 on error
 */
int64_t argint(int n, uint64_t *ip)
{
    if (n < 0 || n > 5) {
        cprintf("argint: invalid argument number %d\n", n);
        return -1;
    }
    struct proc *proc = myproc();
    *ip = proc->tf->regs[n];
    return 0;
}

/*
 * Assign the nth argument in the system call to PP as a pointer and check that the pointer is within the process valid bounds
 */
int64_t argptr(int n, char **pp, int size)
{
    uint64_t i;
    if (argint(n, &i) < 0) 
        return -1;

    struct proc *p = myproc();
    if(i + size > p->sz) 
        return -1;
        
    *pp = (char *)i;
    return 0;
}

/*
 * The nth argument in the system call is used as a pointer and get the string
 */
int64_t argstr(int n, char **pp)
{
    uint64_t addr;
    if (argint(n, &addr) < 0) 
        return -1;
    return fetchstr(addr, pp);
}
