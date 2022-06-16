/**
 * @file types.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Atomic variable structure
 */
typedef struct {
    int64_t counter;
} atomic_t;

/*
 * Generic linked list head structures
 */
struct list_head {
    struct list_head *next, *prev;
};

#endif /* TYPES_H */