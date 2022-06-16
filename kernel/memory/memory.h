/**
 * @file memory.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MEMORY_H
#define MEMORY_H

#include "include/types.h"
#include "../arch/aarch64/mmu.h"
#include "../arch/aarch64/board/raspi3/memlayout.h"

#define PAGE_USED       (1 << 0)
#define PAGE_KERNEL     (1 << 1)
#define TOTAL_PAGES_N   (TOTAL_STOP / PGSIZE)   // 262,144 pages

typedef uint64_t pg_idx_t;

struct pg_range {
    pg_idx_t begin, end;
};

struct page {
    uint64_t flags;         // Set of flag bits
    int32_t _mapcount;      // Records the number of references from the User space, is also used for RMAP reverse mapping
    int32_t _refcount;      // Records the number of references from the kernel space
    struct list_head lru;   // It is mainly used in LRU linked list algorithm of page reclamation
    void *virtual_address;  // Point to the virtual address of the page. In the case of high memory, which does not map linearly to kernel space, this value is NULL  
};

extern struct page *pages;
extern struct pg_range free_zone;

/*
 * Set of operating functions for page
 */
inline static void set_page_used(struct page *page)
{
    page->flags |= PAGE_USED;
}

inline static void set_page_unused(struct page *page)
{
    page->flags &= ~PAGE_USED;
}

inline static void set_page_mapcount(struct page *page, int32_t count)
{
    page->_mapcount = count;
}

inline static int32_t get_page_mapcount(struct page *page)
{
    return page->_mapcount;
}

inline static void set_page_refcount(struct page *page, int32_t value)
{
    page->_refcount = value;
}

inline static int32_t get_page_refcount(struct page *page)
{
    return page->_refcount;
}

inline static int is_page_used(struct page *page)
{
    return page->flags & PAGE_USED;
}

inline static void set_page_kernel(struct page *page)
{
    page->flags |= PAGE_KERNEL;
}

inline static void clear_page_kernel(struct page *page)
{
    page->flags &= ~PAGE_KERNEL;
}

inline static int is_page_kernel(struct page *page)
{
    return page->flags & PAGE_KERNEL;
}

inline static pg_idx_t PHY2PFn(void *ptr)
{
    return (uint64_t)(ptr) >> PAGE_SHIFT;
}

inline static void *PFn2PHY(pg_idx_t idx)
{
    return (void *)(idx << PAGE_SHIFT);
}

inline static struct page *PFn2PAGE(pg_idx_t idx)
{
    return &pages[idx];
}

inline static pg_idx_t PAGE2PFn(struct page *page)
{
    return (pg_idx_t)(page - pages);
}

#endif /* MEMORY_H */