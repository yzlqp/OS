/**
 * @file vm.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef VM_H
#define VM_H

#include <stdbool.h>
#include <stddef.h>
#include "arch/aarch64/arm.h"
#include "memory.h"

struct proc;  // forward declaration

/**
 * @brief  Assign a page to hold the page table
 * @retval 
 */
pagetable_t alloc_pagetable(void);

/**
 * @brief  Given a page table and a virtual address, return its physical address Only for user - mode page tables
 * @param  pagetable_ptr: 
 * @param  va: 
 * @retval 
 */
uint64_t walkaddr(pagetable_t pagetable_ptr,uint64_t va);

/**
 * @brief  Adding a mapping to the kernel page table does not refresh the TLB
 * @param  kernel_pagetable_ptr: 
 * @param  va: 
 * @param  pa: 
 * @param  size: 
 * @param  perm: 
 * @retval None
 */
void kvmmap(pagetable_t kernel_pagetable_ptr,uint64_t va, uint64_t pa, uint64_t size, uint32_t perm);

/**
 * @brief  Creates a VA-PA mapping on the specified page table
 * @param  pagetable_ptr: 
 * @param  va: 
 * @param  pa: 
 * @param  size: 
 * @param  perm: 
 * @retval 
 */
int mappages(pagetable_t pagetable_ptr,uint64_t va, uint64_t pa, uint64_t size, int perm);

/**
 * @brief  Create an empty user page table and return 0 if there is no space
 * @retval 
 */
pagetable_t uvmcreate(void);

/**
 * @brief  Place the initial user-mode program code at address 0 of the given page table, 
 * @param  pagetable: 
 * @param  *src: 
 * @param  sz: 
 * @retval None
 */
void uvminit(pagetable_t pagetable, uint8_t *src, int64_t sz);

/**
 * @brief  Allocates virtual memory for the process, no alignment required
 * @param  pagetable: 
 * @param  oldsz: 
 * @param  newsz: 
 * @retval returns the new size on success, 0 otherwise
 */
uint64_t uvmalloc(pagetable_t pagetable,uint64_t oldsz, uint64_t newsz);

/**
 * @brief  The mapping of the virtual address is cancelled for user space
 * @retval 
 */
uint64_t uvmdealloc(pagetable_t, uint64_t, uint64_t);

/**
 * @brief  Given the page table of a parent process, copy the memory of this process into the page table of the child process
 * @retval Both content and page table entries are copied, 0 indicates success -1 indicates failure
 */
int uvmcopy(pagetable_t, pagetable_t, uint64_t);

/**
 * @brief  Free Memory in user mode, then release the page that holds the page table
 * @retval None
 */
void uvmfree(pagetable_t, uint64_t);

/**
 * @brief  The mapping of the virtual address is cancelled
 * @param  pagetable: 
 * @param  va: 
 * @param  npages: 
 * @param  do_free: 
 * @retval None
 */
void unmunmap(pagetable_t pagetable,uint64_t va,uint64_t npages, int do_free);

/**
 * @brief  Marks a PTE as user unavailable, Used for stack memory protection
 * @param  *pgdir: 
 * @param  *va: 
 * @retval None
 */
void uvmclear(uint64_t *pgdir, uint8_t *va);

/**
 * @brief  Copy from kernel to user.
 * Copy len bytes from src to virtual address dstva in a given page table.(user pagetable)
 * @param  pagetable: 
 * @param  dstva: 
 * @param  src: 
 * @param  len: 
 * @retval Return 0 on success, -1 on error.
 */
int copyout(pagetable_t pagetable, uint64_t dstva, char* src,  uint64_t len);

/**
 * @brief  Copy data from the user to the kernel
 * @param  pagetable: 
 * @param  *dst: 
 * @param  srcva: 
 * @param  len: 
 * @retval 
 */
int copyin(pagetable_t pagetable, char *dst, uint64_t srcva, uint64_t len);

/**
 * @brief  Copies a string ending in '\0' from process memory to the kernel
 * @param  pagetable: 
 * @param  *dst: 
 * @param  srcva: 
 * @param  max: 
 * @retval 0 means success and -1 means failure
 */
int copyinstr(pagetable_t pagetable, char *dst, uint64_t srcva, uint64_t max);

/**
 * @brief  Switches the user page table for this core to the page table for the specified process
 * @param  *p: 
 * @retval None
 */
void uvmswitch(struct proc *p);

#endif /* VM_H */