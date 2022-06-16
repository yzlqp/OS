/**
 * @file kalloc.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef KALLOC_H
#define KALLOC_H

#include <stddef.h>
#include "memory.h"

/**
 * @brief  Initialize memory management system
 * @retval None
 */
void alloc_init(void);

/**
 * @brief  Prints memory management information for Buddy system, 
 * Displays the current memory management status of the system
 * @retval None
 */
void log_alloc_system_info(void);

/**
 * @brief  Memory allocation function in kernel
 * @param  size: Size of requested bytes
 * @retval Returns the start virtual address assigned. 
 * NULL indicates that the assignment failed.
 */
void *kalloc(size_t size);

/**
 * @brief  The assigned virtual address is released
 * @param  *ptr: Points to the virtual address to be released
 * @retval None
 */
void kfree(void *ptr);

/**
 * @brief  Request to assign physical page
 * @param  **page: Point the address of the page 
 * @param  *pfn: Point a page frame number
 * @param  pages_n: The number of page frames
 * @retval 0 means success and -1 means failure
 */
int kalloc_pages(struct page **page, pg_idx_t *pfn, int pages_n);

/**
 * @brief  Request to assign a physical page
 * @param  **page: Point the address of the page 
 * @param  *pfn: Point a page frame number
 * @retval 0 means success and -1 means failure
 */
int kalloc_one_page(struct page **page, pg_idx_t *pfn);

/**
 * @brief  Release the allocated physical page
 * @param  pfn: Points to the page frame number to release
 * @retval None
 */
void kfree_page(pg_idx_t pfn);

#endif /* KALLOC_H */