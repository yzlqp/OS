/**
 * @file spinlock.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>
#include <stdbool.h>

/* 
 * Mutual exclusion lock
 */
struct spinlock {
    bool locked;        // Is the lock held?

    // For debugging
    const char *name;   // The lock name
    struct cpu *cpu;    // The cpu holding the lock
};

/**
 * @brief  Check whether the current CPU has this spin lock, Interrupts must be turned off when this function is called
 * @param  *lock: Pointer to a lock structure
 * @retval Hold is 1, don't hold is 0
 */
bool is_current_cpu_holding_spin_lock(struct spinlock *lock);

/**
 * @brief  Acquire the spin lock, Close local interrupt
 * @param  *lock: Pointer to a lock structure
 * @retval None
 */
void acquire_spin_lock(struct spinlock *lock);

/**
 * @brief  Releases the specified spin lock
 * @param  *lock: Pointer to a lock structure
 * @retval None
 */
void release_spin_lock(struct spinlock *lock);

/**
 * @brief Initialize the spin lock
 * @param  *lock: Pointer to a spinlock structure
 * @param  *name: The name of the spinlock
 * @retval None
 */
void init_spin_lock(struct spinlock *lock, const char *name);

/**
 * @brief  Turn off all interrupt exceptions (DAIF) and increase the lock depth by 1
 * @retval None
 */
void push_off(void);

/**
 * @brief  Turn on all interrupt exceptions (DAIF) and decrease the lock depth by 1
 * @retval None
 */
void pop_off(void);

#endif /* SPINLOCK_H */