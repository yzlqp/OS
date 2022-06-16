/**
 * @file spinlock.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "spinlock.h"
#include "interrupt/interrupt.h"
#include "proc/proc.h"
#include "../printf.h"

/*
 * Initialize the spin lock
 */
void init_spin_lock(struct spinlock *lock, const char *name)
{
    lock->locked = false;
    lock->name = name;
    lock->cpu = NULL;
}

/*
 * Turn off all interrupt exceptions (DAIF) and increase the lock depth by 1
 */
void push_off(void)
{
    bool old = is_interrupt_enabled();
    disable_interrupt();

    if (mycpu()->depth_spin_lock == 0) {
        mycpu()->is_interrupt_enabled = old;
    }
    mycpu()->depth_spin_lock += 1;
}

/*
 * Turn on all interrupt exceptions (DAIF) and decrease the lock depth by 1
 */
void pop_off(void)
{
    struct cpu *cpu = mycpu();
    if (is_interrupt_enabled()) {
        panic("pop_off: interruptible before pop_off");
    }
    if (cpu->depth_spin_lock < 1) {
        panic("pop_off: depth_spin_lock < 1");
    }
    cpu->depth_spin_lock -= 1;
    if (cpu->depth_spin_lock == 0 && cpu->is_interrupt_enabled) {
        enable_interrupt();
    }
}

/*
 * Acquire the spin lock, Close local interrupt
 * Loops (spins) until the lock is acquired.
 */
void acquire_spin_lock(struct spinlock *lock)
{
    // disable interrupts to avoid deadlock
    push_off();
    if (is_current_cpu_holding_spin_lock(lock))
        panic("acquire_spin_lock: the lock (%s) is already held by %lu\n", lock->name, cpuid());

    while (lock->locked || __atomic_test_and_set(&lock->locked, __ATOMIC_ACQUIRE));

    // memory barriorit, tells the compiler and CPU to not reorder loads or stores across the barrier
    __sync_synchronize();
    // Record info about lock acquisition for holding() and debugging.
    lock->cpu = mycpu(); 
}

/*
 * Releases the specified spin lock
 */
void release_spin_lock(struct spinlock *lock)
{
    if (!is_current_cpu_holding_spin_lock(lock))
        panic("release_spin_lock: the lock (%s) held by %lu can't be released by %lu\n", lock->name, lock->cpu->cpuid, cpuid());

    lock->cpu = NULL;
    __sync_synchronize();
    __sync_lock_release(&lock->locked);
    pop_off();
}

/*
 * Check whether the current CPU has this spin lock
 */
bool is_current_cpu_holding_spin_lock(struct spinlock *lock)
{
    return lock->locked && lock->cpu == mycpu();
}
