/**
 * @file poison.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef POISON_H
#define POISON_H

/* 
 * for list.h 
 * need to provide an invalid pointer value that if accessed will generate a segment error for debugging purposes
 * For AARCH64, the high significant bits of the virtual address must be all 0s or all 1s
 */
#define ILLEGAL_POINTER_VALUE 0xdead000000000000

#define LIST_POISON1   ((void *)(0x01 + ILLEGAL_POINTER_VALUE))
#define LIST_POISON2   ((void *)(0x10 + ILLEGAL_POINTER_VALUE))

#endif /* POISON_H */