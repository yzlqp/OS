/**
 * @file mmu.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARM_MMU_H
#define ARM_MMU_H

#define PAGE_SHIFT      12
#define PGSIZE          (1 << PAGE_SHIFT)
#define STACK_SIZE      (PGSIZE * 2)

#define PGROUNDUP(sz)   (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a)  (((a)) & ~(PGSIZE - 1))

/* 
 * In the MMU page entry of AARCH64, in 4K page granularity, the format is as follows
 * |63 --- 52|51 --- 48|47 --- 12|11 --- 2|1|0|
 * [63:52] Upper attributes
 * [11:2]  Lower attributes
 * [47:12] Output address (either the address of the next level page table or the address of the physical page frame)  
 * 
 * In the case of Stage1 translation, we should pay attention to the following
 * [54] UXN/XN Execute-never bit That is, at set 1, this memory content is disabled when EL0 privileges are executed
 * [53] PXN Priviledged execute-never bit When this location is 1, EL1 privileges are prohibited from executing this memory content
 * [10] AF Access flag When this position is 0, accessing this bit will raise an exception. This bit can be used for page reclamation, set 1 if you don't want this function
 * [9:8] SH Shareability field Controlling the Shared Domain00 Non-share 10 Outer-Share 11 Inner-Share
 * [7:6] AP Access permissions Controlling read and write Permissions
 *            AP      EL1     EL0
 *            00      R/W     None
 *            01      R/W     R/W
 *            10      R       None
 *            11      R       R
 * [4:2] AttrIdx Controls the index of memory attributes pointing to the eight elements of register MAIR_ELx,It's up to the user to decide which attributes are needed in the register and where
 * [1:0] Page table type, see MM_TYPE_INVALID MM_TYPE_BLOCK MM_TYPE_TABLE(MM_TYPE_PAGE)
 */
#define MM_TYPE_INVALID 0b00
#define MM_TYPE_BLOCK   0b01
#define MM_TYPE_TABLE   0b11
#define MM_TYPE_PAGE    0b11

/*
 * Memory Attributes control the Memory type and cache strategy of the Memory region that corresponds to this page entry
 */  
#define MA_DEVICE_nGnRnE_Flags  0x0     // Device memory Prohibit to gather(non Gathering) Ban rearrangement(non re-order) Disallow writing ACK ahead of time(Early Write Acknowledgement)
#define MA_MEMORY_Flags         0xFF    // The common memory Use write back, write assignment, read assignment(Fastest form, various cache buffs)
#define MA_MEMORY_NoCache_Flags 0x44    // The common memory  Disallow all caching policies

/*
 * MAIR_ELx There are eight Memory Attributes that can be placed, but we only need these three
 */
#define MA_DEVICE_nGnRnE        0
#define MA_MEMORY               1
#define MA_MEMORY_NoCache       2

#define PTE_AIDX_DEVICE_nGnRn       (MA_DEVICE_nGnRnE << 2)
#define PTE_AIDX_MEMORY             (MA_MEMORY << 2)
#define PTE_AIDX_MEMORY_NOCACHE     (MA_MEMORY_NoCache << 2)

/*
 * That's the value that we're going to put in the MAIR EL1 register
 */
#define MAIR_VALUE \
    (MA_DEVICE_nGnRnE_Flags << (8 * MA_DEVICE_nGnRnE)) | (MA_MEMORY_Flags << (8 * MA_MEMORY)) | (MA_MEMORY_NoCache_Flags << (8 * MA_MEMORY_NoCache))

#define AP_RW           0           // read and write
#define AP_RO           1           // read only
#define AP_PRIVILEGED   0           // Only the privileged level is available
#define AP_UNPREVILEGED 1           // Allow EL0 access

#define PTE_VALID       1
#define PTE_BLOCK       MM_TYPE_BLOCK
#define PTE_TABLE       MM_TYPE_TABLE
#define PTE_PAGE        MM_TYPE_PAGE
#define PTE_KERNEL      (AP_PRIVILEGED << 6)
#define PTE_USER        (AP_UNPREVILEGED << 6)
#define PTE_RW          (AP_RW << 7)
#define PTE_RO          (AP_RO << 7)
#define PTE_SH          (0b11 << 8)  // For SMP systems, all Settings are inner-share
#define PTE_AF_USED     (1 << 10)
#define PTE_AF_UNUSED   (0 << 10)

// Gets the physical address in the page entry, We need the content of the [47:12] scope
#define PTE_ADDR(pte)   ((uint64_t)(pte) & 0xfffffffff000)
// Gets flags in the page table entry, Something other than [47:12] is required
#define PTE_FLAG(pte)   ((uint64_t)(pte) & (~0xfffffffff000))

// Default page entry, memory with the cache, inner share, Privileged read and write and AF is set
#define PTE_NORMAL_MEMORY (MM_TYPE_BLOCK | PTE_AIDX_MEMORY | PTE_SH | PTE_AF_USED | PTE_RW)
// Device memory page entry
#define PTE_DEVICE_MEMORY (MM_TYPE_BLOCK | PTE_AIDX_DEVICE_nGnRn | PTE_AF_USED)
// Points to the next level page table
#define PTE_TABLE          MM_TYPE_TABLE

/*
 * Next configure the TCR translation control register
 * The format of TCR ELx is as follows  
 * [63:39] RES0
 * [38] TBI1 Top Byte ignored Indicates whether the top byte of the address is used to match the address of the TTBR1_EL1 region (set 0) or is ignored and used to mark the address (set 1)
 * [37] TBI0 The same as TBI1 is simply TTBR0 EL1 area address
 * [36] ASID Size If 1 is set to 16 bits, 0 is 8 bits. If the implementation does not support 16 bits, this bit is regarded as RES0
 * [35] RES0
 * [34:32] Middle physical address size
 *        000 32bits 4G
 *        001 26bits 64G
 *        010 40bits 1TB
 *        011 42bits 4TB
 *        100 44bits 16TB
 *        101 48bits 256TB
 *        The remaining values are considered 101, but the schema may change in the future
 * [31:30] TG1 TTBR1_EL1 granularity
 *        01 16K
 *        10 4K
 *        11 64K
 * [29:28] SH1 TTBR1_EL1 Shareability
 *        01 Non-shareable
 *        10 Outer-shareable
 *        11 Inner-shareable
 * [27:26] ORGN1 TTBR1_EL1 Outer cacheability
 *        00 Outer Non-cacheable
 *        01 Outer Write back, write assignment
 *        10 Outer Write through
 *        11 Outer Write back, None write assignment
 * [25:24] IRGN1 TTBR1_EL1 Inner cacheability 
 *         Encoding is the same as ORNG1
 * [23] EPD1 When TTBR1 is used, if this bit is set to 1, translation fault will occur when TLB miss occurs when traversing the page table
 * [22] If the value is 0, ttbr0_el1 .ASID defines the ASID. If the value is 1, ttbr1_el1 .ASID defines the ASID
 * [21:16] T1SZ TTBR1_EL1 has a memory region of 64 - T1SZ, For example, if we need a 48-bit virtual address space, we need to set TxSZ to 64-48
 * [15:14] TG0 TTBR0_EL1 granularity
 *          00 4K
 *          01 64K
 *          10 16K
 * [13:12] SH0 TTBR0_EL1 Shareability The same encoding as SH1 [29:28]
 * [11:10] ORGN0 TTBR0_EL1 Outer cacheability The encoding is the same as ORGN1
 * [9:8] IRGN0
 * [7] EPD0 see EPD1
 * [6] RES0
 * [5:0] T0SZ
 */
#define TCR_IPS         (0 << 32)
#define TCR_TG1_4K      (0b10 << 30)
#define TCR_SH1_INNER   (0b11 << 28)
#define TCR_ORGN1_IRGN1_WRITEBACK_WRITEALLOC ((0b01 << 26) | (0b01 << 24))
#define TCR_TG0_4K      (0 << 14)
#define TCR_SH0_INNER   (0b11 << 12)
#define TCR_ORGN0_IRGN0_WRITEBACK_WRITEALLOC ((0b01 << 10) | (0b01 << 8))
#define TCR_VALUE                                                        \
    (TCR_IPS |                                                           \
     TCR_TG1_4K | TCR_SH1_INNER | TCR_ORGN1_IRGN1_WRITEBACK_WRITEALLOC | \
     TCR_TG0_4K | TCR_SH0_INNER | TCR_ORGN0_IRGN0_WRITEBACK_WRITEALLOC)

/*
 * In System Control Register enable MMU 
 */
#define SCTLR_MMU_ENABLED               (1 << 0)


/*
 * Something about page table entries
 * See Chapter 12 of ARM Cortex-A Series Programmer's Guide for ARMv8-A
 * and Chapter D4(D4.2, D4.3) of Arm Architecture Reference Manual Armv8, for
 * Armv8-A architecture profile.
 *
 * A virtual address 'va' has a four-part structure as follows:
 * +-----9-----+-----9-----+-----9-----+-----9-----+---------12---------+
 * |  Level 0  |  Level 1  |  Level 2  |  Level 3  | Offset within Page |
 * |   Index   |   Index   |   Index   |   Index   |                    |
 * +-----------+-----------+-----------+-----------+--------------------+
 *  \PTX(va, 0)/\PTX(va, 1)/\PTX(va, 2)/\PTX(va, 3)/
 */
#define ENTRYSZ         (PGSIZE / 8)

/*
 * Gets a segment of the specified virtual address
 */
#define PXMASK          0x1FF  
#define PXSHIFT(level)  (PAGE_SHIFT + (9 * (3 - level)))
#define PX(level, va)   ((uint64_t)(va) >> (PXSHIFT(level)) & PXMASK)

#endif /* ARM_MMU_H */