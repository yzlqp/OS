/**
 * @file vm.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "vm.h"
#include "arch/aarch64/mmu.h"
#include "arch/aarch64/arm.h"
#include "printf.h"
#include "kalloc.h"
#include "lib/string.h"
#include "../proc/proc.h"

/*
 * Check whether the maximum 16-bit virtual address is valid 
 * [63:48] All 1s or all 0s are valid
 */
static inline bool _check_MSB_valid(uint64_t va)
{
    uint64_t most_significant_bits = va >> (64 - 16) & 0xFFFF;
    return (most_significant_bits == 0xFFFF) || (most_significant_bits == 0);
}

/*
 * Given a page table, and a virtual address.  Return the CORRESPONDING PTE if the virtual address exists
 * If it does not exist, create a mapping if alloc is true. Return null if alloc is false
 * Rlevel is the level of the page table returned (in the case of four-level page tables, there may be 1,2, or 3 values).
 * This value can be set to NULL if you do not care
 */
static pte_t *walk(pagetable_t pagetable_ptr, uint64_t va, bool alloc, int *rlevel)
{
    if (!_check_MSB_valid(va)) {
        panic("walk: invalid virtual address");
    }
    int level;
    for (level = 0; level < 3; ++level) {
        pte_t *pte = &pagetable_ptr[PX(level, va)];
        if ((*pte & PTE_VALID) != 0) {
            // Address valid
            pagetable_ptr = (pagetable_t)PA2VA(PTE_ADDR(*pte));
            if((*pte & 0b11) == PTE_BLOCK) 
                break;
        } else {
            // Address invalid
            if(!alloc) 
                return NULL;
            // Create a mapping if alloc is true
            if ((pagetable_ptr = (pagetable_t)kalloc(PGSIZE)) == NULL) {
                return NULL;
            }
            // The page clear 0
            memset(pagetable_ptr, 0, PGSIZE);
            *pte = VA2PA(pagetable_ptr) | PTE_PAGE;
        }
    }
    if (rlevel != NULL) {
        *rlevel = level;
    }
    return &pagetable_ptr[PX(level, va)];
}

/*
 * The mapping of the virtual address is cancelled
 */
void unmunmap(pagetable_t pagetable, uint64_t va, uint64_t npages, int do_free)
{
    if ((va % PGSIZE) != 0 )
        panic("unmunmap: invalid virtual address, which is not aligned.\n");

    for (uint64_t a = va; a < va + npages * PGSIZE; a += PGSIZE) {
        pte_t *pte = walk(pagetable, va, 0, NULL);
        if (pte != NULL)
            panic("unmunmap: walk. \n");
        if ((*pte & PTE_VALID) == 0)
            panic("unmunmap: not mapped. \n");
        // If it is not a leaf node, there is only one valid bit and no permission bit 
        if (PTE_FLAG(*pagetable) == PTE_VALID) 
            panic("unmunmap: note a leaf. \n");
        if (do_free) {
            uint64_t pa = PA2VA(PTE_ADDR(*pte));
            kfree((void *)pa);
        }
        *pte = 0;
    }
}

/*
 * Given a page table and a virtual address, return its physical address
 * Only for user - mode page tables
 */
uint64_t walkaddr(pagetable_t pagetable_ptr,uint64_t va)
{
    pte_t* pte;
    if(!_check_MSB_valid(va)) return 0;
    pte = walk(pagetable_ptr,va,false,NULL);
    if(pte == NULL) return 0;
    if((*pte & PTE_VALID) == 0) return 0;
    if((*pte & PTE_USER) == 0) return 0;
    return (uint64_t)PA2VA(PTE_ADDR(*pte));
}

/*
 * Creates a VA-PA mapping on the specified page table
 * Non-zero values represent failure (memory is out of space)
 * Use level 4 page tables, not used for kernel page tables (kernel page tables contain block items)
 */
int mappages(pagetable_t pagetable_ptr, uint64_t va, uint64_t pa, uint64_t size, int perm)
{
    if (size == 0) 
        panic("mappages: size couldn't be zero");
    
    pte_t *pte;
    uint64_t _va = PGROUNDDOWN(va);
    uint64_t last = PGROUNDDOWN(va + size -1);
    while (1) {
        if ((pte = walk(pagetable_ptr, _va, true, NULL)) == NULL) 
            return -1;
        if (*pte & PTE_VALID) {
            panic("mappages: remap");
            return -1;
        }
        *pte = PTE_ADDR(pa) | perm | PTE_VALID | PTE_PAGE | PTE_AF_USED | PTE_SH | PTE_AIDX_MEMORY;
        if(_va == last)
            break;
        _va += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

/*
 * Adding a mapping to the kernel page table does not refresh the TLB
 */
void kvmmap(pagetable_t kernel_pagetable_ptr, uint64_t va, uint64_t pa, uint64_t size, uint32_t perm)
{
    if (mappages(kernel_pagetable_ptr, va, pa, size, perm) != 0) 
        panic("kvmmap: mappages failed");
}

/*
 * Create an empty user page table and return 0 if there is no space
 */
pagetable_t uvmcreate(void)
{
    pagetable_t pagetable;
    pagetable = (pagetable_t)kalloc(PGSIZE);
    if (pagetable==NULL) 
        return NULL;

    memset(pagetable,0,PGSIZE);
    return pagetable;
}

/*
 * Place the initial user-mode program code at address 0 of the given page table, 
 * only for the initial user process, and ensure that sZ is less than the size of a page (this restriction may be removed later).
 */
void uvminit(pagetable_t pagetable, uint8_t *src, int64_t sz)
{
    if (sz >= PGSIZE) 
        panic("uvmminit: more than a page.");

    uint8_t *mem = kalloc(PGSIZE);
    if (mem == NULL) 
        panic("uvminit: memory allocation failed.");

    memset(mem, 0, PGSIZE);
    memmove(mem, src, sz);
    mappages(pagetable, 0, (uint64_t)mem, sz, PTE_USER | PTE_RW | PTE_PAGE);
}

/*
 * Allocates virtual memory for the process, no alignment required, returns the new size on success, 0 otherwise
 */
uint64_t uvmalloc(pagetable_t pagetable,uint64_t oldsz, uint64_t newsz)
{
    if (newsz<oldsz) 
        return oldsz;

    oldsz = PGROUNDUP(oldsz);
    for (uint64_t a = oldsz; a < newsz; a+=PGSIZE) {
        uint8_t *mem = kalloc(PGSIZE);
        if (mem == NULL) 
            uvmdealloc(pagetable,a,oldsz);
        memset(mem,0,PGSIZE);
        if (mappages(pagetable, a, (uint64_t)mem, PGSIZE, PTE_USER | PTE_RW | PTE_PAGE)) {
            kfree(mem);
            uvmdealloc(pagetable, a, oldsz);
            return 0;
        }
    }
    return newsz;
}

/*
 * The mapping of the virtual address is cancelled for user space
 */
uint64_t uvmdealloc(pagetable_t pagetable, uint64_t oldsz, uint64_t newsz)
{
    if(newsz >= oldsz) 
        return oldsz;
    int npages = (oldsz - newsz) / PGSIZE;
    unmunmap(pagetable, newsz, npages, 1);
    return newsz;
}

/* 
 * Given the page table of a parent process, copy the memory of this process into the page table of the child process
 * Both content and page table entries are copied, 0 indicates success -1 indicates failure
 * When the execution fails, all pages allocated during this process are cleaned up
 */
int uvmcopy(pagetable_t old, pagetable_t new, uint64_t sz)
{
    for (uint64_t i = 0; i < sz; i += PGSIZE) {
        pte_t *pte = walk(old, i, false, NULL);
        if (pte == NULL) 
            panic("uvmcopy: pte should exist.\n");
        if (((*pte) & PTE_VALID) == 0) 
            panic("uvmcopy: page note present.\n");
        uint64_t pa = PA2VA(PTE_ADDR(*pte));
        uint64_t flags = PTE_FLAG(*pte);
        uint8_t *mem = kalloc(PGSIZE);
        if (mem == NULL) {
            unmunmap(new, 0, i / PGSIZE, 1);
            return -1;
        }
        memmove(mem, (void *)pa, PGSIZE);
        if (mappages(new, i, (uint64_t)mem, PGSIZE, flags) != 0) {
            kfree(mem);
            unmunmap(new, 0, i / PGSIZE, 1);
            return -1;
        }
    }
    return 0;
}

/* 
 * Free Memory in user mode, then release the page that holds the page table
 */
void uvmfree(pagetable_t pagetable, uint64_t level)
{
    if (pagetable == NULL || level < 0)
        return;
    // The PTE FLAG is 12 bits lower 
    // If it is not equal to zero it means that there is no 4k alignment for this address
    if (((uint64_t)pagetable & 0xFFF)!=0) 
        panic("uvmfree: invalid pte. \n");

    if (level == 0) {
        kfree(pagetable);
        return;
    }

    for (uint64_t i = 0;i < ENTRYSZ; ++i) {
        if (pagetable[i] & PTE_VALID) {
            uint64_t *v = (uint64_t *)PA2VA(PTE_ADDR(pagetable[i]));
            uvmfree(v, level-1);
        }
    }
    kfree(pagetable);
}

/*
 * Marks a PTE as user unavailable
 * Used for stack memory protection
 */
void uvmclear(uint64_t *pgdir, uint8_t *va)
{
    uint64_t *pte = walk(pgdir, (uint64_t)va, 0, NULL);
    if (pte == NULL) 
        panic("uvmclear: failed to get the PTE. \n");

    *pte  &= ~PTE_USER;
}

/*
 * Copy from kernel to user.
 * Copy len bytes from src to virtual address dstva in a given page table.(user pagetable)
 * Return 0 on success, -1 on error.
 */
int copyout(pagetable_t pagetable, uint64_t dstva, char* src,  uint64_t len)
{
    while (len > 0) {
        uint64_t va = PGROUNDDOWN(dstva);
        uint64_t pa = walkaddr(pagetable,va);
        if(pa == 0) return -1;

        uint64_t n = PGSIZE - (dstva - va);
        if(n > len)
            n = len;
        memmove((void*)(pa + (dstva - va)), src, n);
        len -= n;
        src += n;
        dstva = va + PGSIZE;
    }
    return 0;
}

/*
 * Copy data from the user to the kernel
 */
int copyin(pagetable_t pagetable, char* dst, uint64_t srcva, uint64_t len)
{
    while (len > 0) {
        uint64_t va = PGROUNDDOWN(srcva);
        uint64_t pa = walkaddr(pagetable,va);
        if(pa == 0)
            return -1;
        uint64_t n = PGSIZE - (srcva - va);
        if (n > len)
            n = len;
        memmove(dst, (void*)(pa + (srcva - va)), n);
        len -= n;
        dst += n;
        srcva = va + PGSIZE;
    }
    return 0;
}

/*
 * Copies a string ending in '\0' from process memory to the kernel
 * 0 means success and -1 means failure
 */
int copyinstr(pagetable_t pagetable, char *dst, uint64_t srcva, uint64_t max)
{
    int got_null = 0;
    while (got_null == 0 && max > 0) {
        uint64_t va = PGROUNDDOWN(srcva);
        uint64_t pa = walkaddr(pagetable, va);
        if (pa == 0)
            return -1;
        uint64_t n = PGSIZE - (srcva - va);
        if(n > max)
            n = max;
        char *p = (char *)(pa + (srcva - va));
        while (n > 0) {
            if (*p == '\0') {
                *dst = '\0';
                got_null = 1;
                break;
            } else {
                *dst = *p;
            }
            n -= 1;
            max -= 1;
            p += 1;
            dst += 1;
        }
        srcva = va + PGSIZE;
    }
    if (got_null)
        return 0;
    else 
        return -1;
}

/*
 * Assign a page to hold the page table
 */
pagetable_t alloc_pagetable(void)
{
    pagetable_t pgt;
    if ((pgt = kalloc(PGSIZE)) == NULL) 
        return NULL;
        
    memset(pgt, 0, PGSIZE);
    return pgt;
}

/*
 * Switches the user page table for this core to the page table for the specified process
 */
void uvmswitch(struct proc *p)
{
    if (p == NULL)
        panic("uvmswitch: process invalid.\n");
    if (p->kstack == NULL)
        panic("uvmswitch: process' kstack invalid.\n");
    if (p->pagetable == NULL)
        panic("uvmswitch: process' pagetable invalid.\n");

    lttbr0(VA2PA(p->pagetable));
}