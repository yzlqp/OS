/**
 * @file proc.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef PROC_H
#define PROC_H
#include "include/stdint.h"
#include "include/param.h"
#include "../file/file.h"
#include "arch/aarch64/arm.h"
#include "arch/aarch64/include/context.h"
#include "memory/vm.h"
#include "sync/spinlock.h"
#include "arch/aarch64/include/trapframe.h"

enum process_state {
    UNUSED,
    EMBRYO,             
    SLEEPING,
    RUNNABLE,
    RUNNING,
    ZOMBIE
};

enum task_flags {
    PF_KTHREAD = 1 << 0,
};

/*
 * Per-process state
 */
struct proc {
    struct spinlock lock;

    // p->lock must be held when using these:
    enum process_state state;   // Process state
    void *chan;                 // If non-zero, sleeping on chan
    int killed;                 // If non-zero, have been killed
    int xstate;                 // Exit status to be returned to parent's wait
    int pid;                    // process id
    
    // wait_lock must be held when using this:
    struct proc *parent;        // Points to the parent of the process

    // these are private to the process, so p->lock need not be held.
    uint8_t *kstack;            // Virtual address of kernel stack
    uint64_t sz;                // Size of process memory (bytes)
    pagetable_t pagetable;      // User page table
    struct trapframe *tf;       // Used when a process makes a system call
    struct context context;     // Hardware context, swtch() here to run process
    struct file *ofile[NOFILE]; // Open files
    struct inode *cwd;          // Current working directory inode
    char name[16];              // Process name for debugging

    // Newly added
    enum task_flags flags;      // Process flag bit
    long count;                 // Time slice for process scheduling
    int priority;               // Process priority
};

struct process_table {
    struct proc proc[NPROC];
};

/*
 * Per-CPU state
 * The process structure of the CPU running process, the context of the CPU scheduling thread, and information used to manage interrupts 
 */
struct cpu {
    struct context context;     // swtch() here to enter scheduler()
    struct proc *proc;          // The process running on this cpu, or null
    bool is_interrupt_enabled;  // Were interrupts enabled before push_off()
    int depth_spin_lock;        // Depth of push_off() nesting
    int cpuid;                  // for debug
};

#define INIT_TASK(task)     \
{                           \
    .state = 0,             \ 
    .pid = 0,               \
    .flags = PF_KTHREAD,    \
    .priority = 1,          \
}

union task_union {
    struct proc proc;
    unsigned long stack[(8 * 1024) / sizeof(long)];
};

/**
 * @brief Obtain the current CUP ID
 * Be careful to use it with interrupts off, otherwise the thread may be switched to another core before the value is returned  
 * Must be called with interrupts disabled, to prevent race with process being moved to a different CPU.
 * @return uint64_t Returns the CPUID, if the value of register [7:0] is 0, it represents the first CPU core
 */
static inline uint64_t cpuid()
{
    return r_mpidr() & 0xFF;
}

/**
 * @brief  Return this CPU's cpu struct, Interrupts must be disabled
 * @retval struct cpu* A pointer to the CPU structure
 */
struct cpu *mycpu(void);

/**
 * @brief  Keep interrupt off while fetching process structure.  
 * Interrupts can be closed when the process structure is removed 
 * @retval Return the current struct proc *, or zero if none
 */
struct proc *myproc(void);

/**
 * @brief  Wake up other cores
 * @retval None
 */
void init_awake_ap_by_spintable();

/**
 * @brief  Initialize the CPU array
 * @retval None
 */
void init_cpu_info();

/**
 * @brief  Initialize the process management subsystem
 * @retval None
 */
void proc_init();

/**
 * @brief  Find the next kernel thread to execute, and SWTCH into that thread
 * @retval None
 */
void scheduler(void);

/**
 * @brief  Initialize the init process 
 * @retval None
 */
void init_user();

/**
 * @brief  Exit the current process. Does not return.
 * @param  status: Specifies the exit status
 * @retval None
 */
void exit(int status);

/**
 * @brief  Puts the current process to sleep and frees the CPU
 * @param  chan: conditional variable
 * @param  *lk: A pointer to a spinlock
 * @retval None
 */
void sleep(void *chan, struct spinlock *lk);

/**
 * @brief  Wake up some processes on a wait queue with state set to runnable
 * @param  chan: conditional variable
 * @retval None
 */
void wakeup(void *chan);

/**
 * @brief  Kill the process with the given pid.
 * @param  pid: Pid of the process to kill
 * @retval int32_t Returns 0 on success, -1 on failure
 */
int32_t kill(int pid);

/**
 * @brief  Create a process, return child’s PID.
 * @retval child’s PID if > 0
 */
int32_t fork(void);

/**
 * @brief  Wait for a child process to exit and return its pid.
 * @param  *xstate: A pointer to the exit state
 * @retval Return -1 if this process has no children.
 */
int32_t wait(int64_t *xstate);

/**
 * @brief  Give up the CPU for one scheduling round.
 * @retval None
 */
void yield(void);

/**
 * @brief  Adjust the virtual address space of a process
 * @param  n: Number of bytes to add or subtract
 * @retval 0 means success and -1 means failure
 */
int32_t growproc(int64_t n);

#endif /* PROC_H */