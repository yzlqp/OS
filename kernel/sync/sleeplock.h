/**
 * @file sleeplock.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-03-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef SLEEPLOCK_H
#define SLEEPLOCK_H

#include "spinlock.h"

/*
 * Long-term locks for processes
 */
struct sleeplock {
    uint32_t locked;     // Is the lock held?
    struct spinlock lk;  // spinlock protecting this sleep lock
    
    // Fields used for debugging
    char *name;         // The name of the lock
    int pid;            // The process that holds the lock
};

/**
 * @brief  Initialize the sleep lock structure
 * @param  lock: Pointer to a lock structure
 * @param  *name: Lock name for debugging
 * @retval None
 */
void init_sleep_lock(struct sleeplock *lock, char *name);

/**
 * @brief  Acquire a sleep lock
 * @param  *lock: Pointer to a lock structure
 * @retval None
 */
void acquire_sleep_lock(struct sleeplock *lock);

/**
 * @brief  Release a sleep lock, Wake up blocked process on the lock
 * @param  *lock: Pointer to a lock structure
 * @retval None
 */
void release_sleep_lock(struct sleeplock *lock);

/**
 * @brief  Determines whether this Sleeplock is held by the current CPU 
 * @param  *lock: Pointer to a lock structure
 * @retval Hold is 1, don't hold is 0
 */
bool is_current_cpu_holing_sleep_lock(struct sleeplock *lock);

#endif /* SLEEPLOCK_H */