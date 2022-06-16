/**
 * @file memlayout.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#define PHYSTOP      0x3F000000 // After this address is the MMIO region, that is, the available physical memory range is from 0 to 0x3eFFFFFF, a total of 1008M
#define TOTAL_STOP   0x40000000

/*
 * In the case of the 48-bit valid virtual address, the address space is divided into two parts  
 * 0x0000_0000_0000_0000 to 0x0000_ffff_ffff_ffff   
 * 0xffff_0000_0000_0000 to 0xffff_ffff_ffff_ffff
 * We will place the kernel in a high address area, so KERNEL_BASE is defined for this  
 */
#define KERNEL_BASE 0xFFFF000000000000

#ifndef __ASSEMBLER__
#include <stdint.h>
// If given an identity map of a high address area virtual address, we convert it to a physical address  
#define VA2PA(a) (((uint64_t)(a)) - KERNEL_BASE)
#define PA2VA(a) (((uint64_t)(a)) + KERNEL_BASE)
#endif

#endif /* MEMLAYOUT_H */