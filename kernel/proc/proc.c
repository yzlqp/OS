/**
 * @file proc.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "proc.h"
#include "include/param.h"
#include "../memory/kalloc.h"
#include "../memory/vm.h"
#include "../lib/string.h"
#include "../printf.h"
#include "../interrupt/interrupt.h"
#include "../fs/fs.h"
#include "../fs/log.h"

struct process_table process_table;
struct cpu cpus[NCPU];
int nextpid = 1;
static struct proc *initproc;
static struct spinlock pid_lock;
static struct spinlock wait_lock;
static volatile uint64_t *_spintable = (uint64_t *)PA2VA(0xD8);

extern void _entry();
extern void user_trapret();
extern void _forkret(struct trapframe *);
extern void swich(struct context *old, struct context *new);
static void freeproc(struct proc *p);
void forkret();

/* 
 * Return this CPU's cpu struct, Interrupts must be disabled
 */
struct cpu *mycpu(void)
{
    return &cpus[cpuid()];
}

/* 
 * Initialize the CPU array
 */
void init_cpu_info()
{
    for (int i = 0; i < NCPU; ++i) {
        cpus[i].depth_spin_lock = 0;
        cpus[i].cpuid = i;
    }
}

/* 
 * Return the current struct proc *, or zero if none
 * Keep interrupt off while fetching process structure.  Interrupts can be closed when the process structure is removed 
 */
struct proc *myproc(void)
{
    push_off();
    struct cpu *cpu = mycpu();
    struct proc *proc = cpu->proc;
    pop_off();
    return proc;
}

/*
 * Initialize the process management subsystem
 */
void proc_init()
{
    init_cpu_info();
    init_spin_lock(&wait_lock, "wait_lock");
    init_spin_lock(&pid_lock, "pid_lock");
    for (struct proc *p = process_table.proc; p < &process_table.proc[NPROC]; ++p) {
        init_spin_lock(&p->lock, "proc");
    }
}

/* 
 * Allocate an available PID
 */
static int allocpid()
{
    int pid;
    acquire_spin_lock(&pid_lock);
    pid = nextpid;
    nextpid += 1;
    release_spin_lock(&pid_lock);
    return pid;
}

/* 
 * Look for an UNUSED state process in the process_table. If it finds one, initialize it and run it.
 * Lock process->lock, NULL is returned if no process is available or memory allocation fails
 * Look in the process table for an UNUSED proc. If found, change state to EMBRYO and initialize
 * state required to run in the kernel. Otherwise return NULL.
 */
static struct proc *allocproc(void)
{
    struct proc *p;
    for (p = process_table.proc; p < &process_table.proc[NPROC]; ++p) {
        acquire_spin_lock(&p->lock);
        if (p->state == UNUSED) {
            goto found;
        } else {
            release_spin_lock(&p->lock);
        }
    }
    return NULL;

found:
    p->pid = allocpid();
    p->state = EMBRYO;

    // Allocate memory space for the kernel stack
    if ((p->kstack = kalloc(KSTACKSIZE)) == NULL) {
        freeproc(p);
        release_spin_lock(&p->lock);
        return NULL;
    }

    // Assign a page table
    if ((p->pagetable = alloc_pagetable()) == NULL) {
        freeproc(p);
        release_spin_lock(&p->lock);
        return NULL;
    }

    // Reserve space on the stack for trapFrame, Top of stack pointer
    uint8_t *sp = p->kstack + KSTACKSIZE;
    // Leave room for trap frame
    sp -= sizeof(*p->tf);
    p->tf = (struct trapframe *)sp;

    // Set up new context to start executing at forkret, which returns to user space.
    memset(&p->context, 0, sizeof(struct context));

    // Construct return address
    p->context.x29 = (uint64_t)sp;
    p->context.sp_el1 = (uint64_t)sp;
    // The kernel thread starts with forkret
    p->context.x30 = (uint64_t)forkret;
    //cprintf("allocproc: process %d allocated.\n", p->pid);
    //cprintf("allocproc: kernel sp: 0x%x\n", sp);
    return p;
}

/* 
 * Find the child of the given process and reset the child's parent to init process
 */
void reparent(struct proc *p)
{
    for (struct proc *child_proc = process_table.proc; child_proc < &process_table.proc[NPROC]; child_proc++) {
        if (child_proc->parent == p) {
            child_proc->parent = initproc;
            wakeup(initproc);
        }
    }
}

/* 
 * free a proc structure and the data hanging from it,
 * including user pages.
 * p->lock must be held.
 */
static void freeproc(struct proc *p)
{
    p->chan = NULL;
    p->killed = 0;
    p->xstate = 0;
    p->pid = 0;
    p->parent = NULL;
    if (p->kstack) {
        kfree(p->kstack);
    }
    p->kstack = NULL;
    p->sz = 0;
    if (p->pagetable) {
        uvmfree(p->pagetable, 4);
    }
    p->pagetable = NULL;
    p->tf = NULL;
    p->name[0] = '\0';
    p->state = UNUSED;
}

/* 
 * A fork child's first scheduling by scheduler() will swtch to forkret
 */
void forkret(void)
{
    volatile static int first = 1;
    struct proc *p = myproc();
    // Still holding p->lock from scheduler
    release_spin_lock(&p->lock);
    if (first) {
        // File system initialization must be run in the context of a
        // regular process (e.g., because it calls sleep), and thus cannot
        // be run from main().
        first = 0;
        fsinit(ROOTDEV);
    }
    _forkret(p->tf);
}

/* 
 * Initialize the init process 
 */
void init_user(void)
{
    extern char _binary_build_user_initcode_bin_start[];
    extern char _binary_build_user_initcode_bin_size[];
    int64_t code_size = (uint64_t)_binary_build_user_initcode_bin_size;
    struct proc *p = allocproc();
    if (p == NULL) 
        panic("init_user: Failed to allocate proc.\n");

    initproc = p;
    p->sz = PGSIZE;
    // Copies the initial user program code into the space of the initial process
    uvminit(p->pagetable, (uint8_t *)_binary_build_user_initcode_bin_start, code_size);
    memset(p->tf, 0, sizeof(*p->tf));
    // Set the user stack location
    p->tf->sp = PGSIZE;
    // Initcode is located at 0 in the process address space, beginning of initcode.S
    p->tf->regs[30] = 0;

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd = namei("/");
    p->state = RUNNABLE;
    release_spin_lock(&p->lock);
}

/* 
 * Switch to scheduler.  Must hold only p->lock and have changed proc->state. Saves and restores
 * intena because intena is a property of this kernel thread, not this CPU. It should
 * be proc->intena and proc->noff, but that would break in the few places where a lock is held but there's no process.
 * 
 * User process PA -> usertrap -> kernel thread TA -> yield(TA) -> Sched (TA) -> SWICH (TA, S) -> scheduling thread S -> 
 * SWTCH (S, TB) -> sched(TB) -> Yield (TB)-> kernel thread TB -> UserTrapret -> User process PB 
 */
void sched(void)
{
    int intena;
    struct proc *p = myproc();

    // First, check again if p->lock is held
    if (!is_current_cpu_holding_spin_lock(&p->lock)) {
        panic("sched: p->lock not locked.\n");
    }
    // Then, check to see if additional locks are held, detect whether there are other locks besides P->lock 
    // Holding other locks and then abandoning the CPU is not allowed, i.e. we cannot hold anything but p- > before calling SWTCH. Locks other than lock
    if (mycpu()->depth_spin_lock != 1) {
        panic("sched: sched locks.\n");
    }
    // The process status and interrupt checks are then completed
    if (p->state == RUNNING) {
        panic("sched: process is running.\n");
    }
    if (is_interrupt_enabled()) {
        panic("sched: interrupt is enabled.\n");
    }

    // Finally, through SWICH, switch to the scheduler that schedules the thread
    intena = mycpu()->depth_spin_lock;
    swich(&p->context, &mycpu()->context);
    mycpu()->depth_spin_lock = intena;
}

/* 
 * Each CPU has one scheduling thread, Per-CPU process scheduler
 * Each CPU calls scheduler() after setting itself up.
 * Scheduler never returns.  It loops, doing:
 * - choose a process to run.
 * - swtch to start running that process.
 * - eventually that process transfers control
 * via swtch back to the scheduler.
 * Find the next kernel thread to execute, and SWTCH into that thread
 * We call SWTCH in Sched and return from scheduler's SWTCH, Then release the p->lock
 * Sched and scheduler represent a pair of coroutines, where the sched of one kernel thread switches to scheduler via SWTCH, 
 * and the scheduler switches back to the sched of another kernel thread via SWTCH 
 */
void scheduler(void)
{
    struct cpu *c = mycpu();
    c->proc = NULL;

    while (1) {
        // Avoid deadlock by ensuring that devices can interrupt
        enable_interrupt();

        for (struct proc *p = process_table.proc; p < &process_table.proc[NPROC]; ++p) {
            acquire_spin_lock(&p->lock);
            if (p->state == RUNNABLE) {
                // Switch to chosen process. It is the process's job
                // to release its lock and then reacquire it
                // before jumping back to us.
                //cprintf("cpu %d to %s\n", cpuid(), p->name);
                p->state = RUNNING;
                c->proc = p;
                uvmswitch(p);
                swich(&c->context, &p->context);

                // Process is done running for now
                // It should have changed its p->state before coming back
                c->proc = NULL;
            }
            release_spin_lock(&p->lock);
        }
    }
}

/* 
 * proc dump for debug
 */
void proc_dump()
{
    static char* states[] = {
        [UNUSED] "UNUSED",  [SLEEPING] "SLEEPING", [RUNNABLE] "RUNNABLE",
        [RUNNING] "RUNNING", [ZOMBIE] "ZOMBIE",
    };

    cprintf("\n====== PROCESS DUMP ======\n");
    for (struct proc *p = process_table.proc; p < &process_table.proc[NPROC]; ++p) {
        if (p->state == UNUSED) 
            continue;
        char *state =
            (p->state >= 0 && p->state < ARRAY_SIZE(states) && states[p->state])
                ? states[p->state]
                : "UNKNOWN";
        cprintf("[%s] %d (%s)\n", state, p->pid, p->name);
    }
    cprintf("====== DUMP END ======\n\n");
}

/* 
 * trapframe dump for debug
 */
void trapframe_dump(struct proc* p)
{
    cprintf("\n====== TRAP FRAME DUMP ======\n");
    cprintf("sp: %d\n", p->tf->sp);
    cprintf("pc: %d\n", p->tf->pc);
    cprintf("pstate: %d\n", p->tf->pstate);
    for (uint64_t i = 0; i < 31; ++i) {
        cprintf("x%d: %d\n", p->tf->regs[i]);
    }
    cprintf("====== DUMP END ======\n\n");
}

/* 
 * Exit the current process. Does not return.
 * An exited process remains in the zombie state until its parent calls wait().
 * The main task is to log the exit status of the calling process, release some resources, 
 * host all children of the current process to init, wake up the parent process of the current 
 * process, set the current process state to ZOMBIE, and finally release the CPU. 
 * 
 * It is important to note that the process that calls exit must hold the parent's lock while 
 * setting its state and waking up the parent, in order to prevent wake loss.  The process that 
 * calls exit also holds its own lock, because the process is ZOMBIE for a while, but we're actually
 * running it, so the parent shouldn't find and release the child.  The same locking rules apply here, 
 * with the parent process followed by the child process, to prevent deadlocks. 
 */
void exit(int status)
{
    struct proc *p = myproc();
    if (p == initproc) 
        panic("init proc exit with status code %d\n", status);

    // Close all open files for the process
    for (int fd = 0; fd < NOFILE; ++fd) {
        if (p->ofile[fd] != NULL) {
            struct file *f = p->ofile[fd];
            fileclose(f);
            p->ofile[fd] = NULL;
        }
    }

    begin_op();
    iput(p->cwd);
    end_op();
    p->cwd = NULL;

    // we need the parent's lock in order to wake it up from wait().
    // the parent-then-child rule says we have to lock it first.
    acquire_spin_lock(&wait_lock);
    // Give any children to init.
    reparent(p);
    // Parent might be sleeping in wait().
    wakeup(p->parent);
    // Set the exit status and call Sched to reschedule
    acquire_spin_lock(&p->lock);
    p->xstate = status;
    p->state = ZOMBIE;
    release_spin_lock(&wait_lock);

    // Jump into the scheduler, never to return.
    sched();
    panic("exit: zombie exit.\n");
}

/* 
 * Puts the current process to sleep and frees the CPU
 * Atomically release lock and sleep on chan.
 * Reacquires lock when awakened.
 */
void sleep(void *chan, struct spinlock *lk)
{ 
    struct proc *p = myproc();

    // Must acquire p->lock in order to
    // change p->state and then call sched.
    // Once we hold p->lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup locks p->lock),
    // so it's okay to release lk.
    acquire_spin_lock(&p->lock);
    release_spin_lock(lk);

    // Go to sleep.
    p->chan = chan;
    p->state = SLEEPING;

    sched();

    // Tidy up.
    p->chan = NULL;
    release_spin_lock(&p->lock);
    acquire_spin_lock(lk);
}

/* 
 * Wake up some processes on a wait queue with state set to runnable
 * Wake up all processes sleeping on chan.
 * Must be called without any p->lock.
 */
void wakeup(void *chan)
{ 
    for (struct proc *p = process_table.proc; p < &process_table.proc[NPROC]; p++) {
        if (p != myproc()) {
            acquire_spin_lock(&p->lock);
            if (p->state == SLEEPING && p->chan == chan) {
                p -> state = RUNNABLE;
            }
            release_spin_lock(&p->lock);
        }
    }
}

/* 
 * Kill the process with the given pid. The victim won't exit until it tries to return to user space (see usertrap() in trap.c).
 * Set the killed flag of the PID process, Returns 0 on success, -1 on failure
 */
int32_t kill(int pid)
{
    for (struct proc *p = process_table.proc; p < &process_table.proc[NPROC]; p++) { 
        acquire_spin_lock(&p->lock);
        if (p->pid == pid) {
            p->killed = 1;
            if (p->state == SLEEPING) {
                // Wake process from sleep()
                p->state = RUNNABLE;
            }
            release_spin_lock(&p->lock);
            return 0;
        }
        release_spin_lock(&p->lock);
    }
    return -1;
}

/* 
 * Create a process, return child’s PID.
 */
int32_t fork(void)
{
    struct proc *parent_proc = myproc();
    struct proc *child_proc = allocproc();
    if (child_proc == NULL) {
        return -1;
    }

    // Copy the parent process memory to the child process
    if (uvmcopy(parent_proc->pagetable, child_proc->pagetable, parent_proc->sz) < 0) {
        freeproc(child_proc);
        release_spin_lock(&child_proc->lock);
        cprintf("fork: copy memory to child process failed.\n");
        return -1;
    }
    child_proc->sz = parent_proc->sz;
    // Copy the register
    *(child_proc->tf) = *(parent_proc->tf);
    // The child process returns 0
    child_proc->tf->regs[0] = 0;
    // The parent process starts file synchronization
    for (int i = 0; i < NOFILE; ++i) {
        if (parent_proc->ofile[i] != NULL) {
            child_proc->ofile[i] = filedup(parent_proc->ofile[i]);
        }
    }
    // Configure the working directory
    child_proc->cwd = idup(parent_proc->cwd);
    strncpy(child_proc->name, parent_proc->name, sizeof(child_proc->name));

    int child_pid = child_proc->pid;
    release_spin_lock(&child_proc->lock);

    acquire_spin_lock(&wait_lock);
    child_proc->parent = parent_proc;
    release_spin_lock(&wait_lock);

    acquire_spin_lock(&child_proc->lock);
    child_proc->state = RUNNABLE;
    release_spin_lock(&child_proc->lock);

    return child_pid;
}

/* 
 * Wait for a child process to exit and return its pid.
 * Return -1 if this process has no children.
 * To let the parent process know that the child process has terminated, 
 * set its running status to ZOMBIE when it exits. Then, the wait process 
 * notices the terminated child process, marks it UNUSED, copies its exit 
 * status, and returns its PID to the parent process
 */
int32_t wait(int64_t *xstate)
{
    struct proc *p = myproc();
    int is_have_child, pid;
    acquire_spin_lock(&wait_lock);

    while (1) {
        // Scan through table looking for exited children.
        is_have_child = 0;
        for (struct proc *np = process_table.proc; np < &process_table.proc[NPROC]; np++) {
            if (np->parent == p) {
                // np->parent can't change between the check and the acquire()
                // because only the parent changes it, and we're the parent.
                acquire_spin_lock(&np->lock);
                is_have_child = 1;
                if (np->state == ZOMBIE) {
                    // Found an exited child process
                    pid = np->pid;
                    if (xstate != NULL) {
                        *xstate = (int64_t)np->xstate;
                    }
                    freeproc(np);
                    release_spin_lock(&np->lock);
                    release_spin_lock(&wait_lock);
                    return pid;
                }
                release_spin_lock(&np->lock);
            }
        }
        // No point waiting if we don't have any children.
        if (is_have_child == 0 || p->killed) {
            release_spin_lock(&wait_lock);
            return -1;
        }
        // Wait for a child to exit.
        sleep(p, &wait_lock);
    }
}

/* 
 * Give up the CPU for one scheduling round.
 * yield the CPU for the current process
 * yield() -> sched() -> swich()
 * The kernel thread is ready to relinquish the CPU, so it first acquirement the process 
 * lock P ->lock because we are changing p->state, and should hold p->lock until it enters the scheduler 
 * After changing the state to RUNNABLE, we call Sched 
 */
void yield(void)
{
    struct proc *p = myproc();
    if (p == NULL) {
        return;
    }
    acquire_spin_lock(&p->lock);
    p->state = RUNNABLE;
    sched();
    release_spin_lock(&p->lock);
}

/* 
 * Adjust the virtual address space of a process
 */
int32_t growproc(int64_t n)
{
    struct proc *p = myproc();
    int32_t sz = p->sz;

    if (n > 0) {
        if((sz = uvmalloc(p->pagetable, sz, sz + n)) == 0) 
            return -1;
    } else if (n < 0) {
        sz = uvmdealloc(p->pagetable, sz, sz + n);
    }
    p->sz = sz;
    return 0;
}

/* 
 * Wake up other cores
 */
void init_awake_ap_by_spintable()
{
    for (int i = 1; i < NCPU; ++i) {
        _spintable[i] = (uint64_t)VA2PA(&_entry);
    }
    asm volatile (
        "dsb st \n\t"
        "sev"
    );
}
