/**
 * @file kalloc.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "kalloc.h"
#include "internal.h"
#include "../printf.h"
#include "include/util.h"
#include "include/list.h"
#include "../sync/spinlock.h"

#define MAX_ORDER 11

extern void pages_init(struct pg_range* range);
static inline void _free_one_page(struct page * page, pg_idx_t pg_idx, order_t order);
static void _free_pages_range(pg_idx_t begin, pg_idx_t end);

/*
 * MAX_ORDER linked lists head for buddy systems
 */
struct free_area {
    struct list_head free_list;
    uint64_t n_free;
};

/*
 * Kernel memory management area (zone) descriptor
 */
struct zone {
    int64_t managed_pages;      // The number of pages in the memory management area managed by the buddy system
    int64_t available_pages;    // Number of memory pages currently available
    struct free_area area[MAX_ORDER];
};

struct zone zone;
struct spinlock alloc_lock;

/*
 * Initialize memory management system
 */
void alloc_init(void)
{
    struct pg_range pg_range;
    pages_init(&pg_range);
    zone.managed_pages = pg_range.end - pg_range.begin;
    zone.available_pages = 0;
    for (int i = 0; i < MAX_ORDER; ++i) {
        struct free_area *free_area_ptr = zone.area + i;
        free_area_ptr->n_free = 0;
        INIT_LIST_HEAD(&free_area_ptr->free_list);
    }
    _free_pages_range(pg_range.begin, pg_range.end);
    init_spin_lock(&alloc_lock, "alloc");
    log_alloc_system_info();
    cprintf("Memory manage system initialized.\n");
}

/*
 * Prints memory management information for Buddy system, Displays the current memory management status of the system
 */
void log_alloc_system_info(void)
{
    cprintf("Buddy system info:\n");
    cprintf("Managed_pages: %d\t Available pages: %d\t bytes: %d\n", zone.managed_pages, zone.available_pages, zone.available_pages * PGSIZE);
    cprintf("Each order:\n");
    int free_pages = 0;
    for (int i = 0; i < MAX_ORDER; ++i) {
        int pages = zone.area[i].n_free << i;
        cprintf("Order: %d\t free: %d\t pages:%d\n", i, zone.area[i].n_free, pages);
        free_pages += pages;
    }
    cprintf("Total free pages: %d\n", free_pages);
}

/*
 * Deletes the specified page from the Buddy system list
 */
static inline void del_page_from_free_list(struct page *page, order_t order)
{
    if (page->lru.next == (void*)0xdead000000000001 || page->lru.next == (void*)0xdead000000000010)
        panic("!!!");
    //cprintf("del_page_from_free_list: order: %d\t", order);
    list_del(&page->lru);
    set_page_order(page, order);
    unset_page_buddy(page);
    zone.area[order].n_free -= 1;
    //cprintf("%d\n", zone.area[order].n_free);
    zone.available_pages -= 1UL << order;
}

/*
 * Add the specified page to the Buddy system list
 */
static inline void add_to_free_list(struct page *page, order_t order)
{
    //cprintf("add_to_free_list: order: %d\t",order);
    struct free_area *area = &zone.area[order];
    list_add(&page->lru, &area->free_list);
    set_page_buddy(page, order);
    //cprintf("zone.area[%d].n_free: %d -> ", order, zone.area[order].n_free);
    area->n_free += 1;
    //cprintf("%d\n", zone.area[order].n_free);
    zone.available_pages += 1UL << order;
}

/*
 * Releases the specified page frame number
 */
static void _free_page(pg_idx_t pfn, order_t order)
{
    //cprintf("free_pages_core: pg_idx: %d, order: %d\n", pfn, order);
    uint32_t n_pages = 1 << order;
    struct page *begin = PFn2PAGE(pfn);
    const struct page * const end = begin + n_pages;
    for (struct page *ptr = begin; ptr < end; ++ptr) {
        clear_page_kernel(ptr);
        set_page_unused(ptr);
        set_page_refcount(ptr, 0);
    }
    _free_one_page(begin, pfn, order);
}

/*
 * Releases pages in the specified range
 */
static void _free_pages_range(pg_idx_t begin, pg_idx_t end)
{
    order_t order;
    while (begin < end) {
        order = MIN(MAX_ORDER - 1UL, __ffsl(begin));
        while ((begin + (1UL << order)) > end) {
            --order;
        }
        _free_page(begin, order);
        begin += (1UL << order);
    }
}

/* 
 * Free a page
 */
static inline void _free_one_page(struct page *page, pg_idx_t pg_idx, order_t order)
{
    while (order < MAX_ORDER - 1) {
        // Looking for his buddy, Verify that buddy is releasable
        pg_idx_t buddy_page_idx = _find_buddy_pfn(pg_idx, order);
        struct page *buddy_page = page + (buddy_page_idx - pg_idx);
        // Exit if Buddy cannot be released
        if (!page_is_buddy(page, buddy_page, order))
            break;
        del_page_from_free_list(buddy_page, order);
        pg_idx_t combined_pg_idx = pg_idx & buddy_page_idx;
        page = page + (combined_pg_idx - pg_idx);
        pg_idx = combined_pg_idx;
        order += 1;
    }
    add_to_free_list(page, order);
}

/*
 * If you acquire a larger chunk of memory than you need, split the chunk and return the excess
 */
static inline void _expand(struct page *page, order_t required_order, order_t current_order)
{
    uint64_t size = 1 << current_order;
    struct free_area *area = &zone.area[current_order];
    while (current_order > required_order) {
        --area;
        --current_order;
        size >>= 1;
        add_to_free_list(page + size, current_order);
    }
}

/*
 * Find the smallest page in the zone's freelist that satisfies the order and pull it out
 */
static inline struct page* _rm_smallest(order_t order)
{
    for (order_t current_order = order; current_order < MAX_ORDER; ++current_order) {
        struct free_area *area = &zone.area[current_order];
        // If there is no memory available for this order, try a higher order
        if (list_is_empty(&area->free_list))
            continue;
        struct page *page = list_first_entry(&area->free_list, struct page, lru);
        // Get it from the available list
        del_page_from_free_list(page, current_order);
        _expand(page, order, current_order);
        return page;
    }
    return NULL;
}

/* 
 * Memory allocation function in kernel
 * Returns the start virtual address assigned. 
 * NULL indicates that the assignment failed.
 */
void *kalloc(size_t size)
{
    if (size == 0)
        return NULL;

    uint64_t pages_n = size / PGSIZE;
    if (size % PGSIZE != 0)
        pages_n += 1;
    
    struct page *page;
    pg_idx_t pg_idx;
    acquire_spin_lock(&alloc_lock);
    if (kalloc_pages(&page, &pg_idx,pages_n) != 0)
        return NULL;

    release_spin_lock(&alloc_lock);
    return (void *)PA2VA(PFn2PHY(pg_idx));
}

/* 
 * Request to assign physical page
 * 0 means success and -1 means failure
 */
int kalloc_pages(struct page **page, pg_idx_t *pfn, int pages_n)
{
    order_t order;

    if ((pages_n&(pages_n-1)) == 0) {
        order = __ffsl(pages_n);
    } else {
        order = __flsl(pages_n) + 1;
    }

    struct page *_page;
    pg_idx_t _pfn;
    _page = _rm_smallest(order);
    set_page_order(_page,order);
    set_page_used(_page);
    _pfn = _page - pages;
    if(_page == NULL)
        return -1;

    *page = _page;
    *pfn = _pfn;

    return 0;
}

/* 
 * Request to assign a physical page
 * 0 means success and -1 means failure
 */
int kalloc_one_page(struct page **page,pg_idx_t *pfn)
{
    return kalloc_pages(page, pfn, 0);
}

/* 
 * Release the allocated physical page
 */
void kfree_page(pg_idx_t pfn)
{
    order_t order = get_page_order(pages + pfn);
    _free_page(pfn, order);
}

/* 
 * The assigned virtual address is released
 */
void kfree(void *ptr)
{
    acquire_spin_lock(&alloc_lock);
    pg_idx_t pfn = PHY2PFn((void *)VA2PA(ptr));
    struct page *page = pages + pfn;
    if (!is_page_used(page))
        panic("kfree: page not used. Double free?.\n");

    set_page_unused(page);
    order_t order = get_page_order(page);
    _free_one_page(page, pfn, order);
    release_spin_lock(&alloc_lock);
}