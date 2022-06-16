#include "mmu.h"
#include "./board/raspi3/memlayout.h"
#include "sysreg.h"

.section ".text._entry"

.global _entry

_entry:
    // To read the current exception level, in this register we need [3:2] (00:EL0 01:EL1 10:EL2 11:EL3)
    mrs     x2, CurrentEL
    and     x2, x2, #0b1100
    cmp     x2, #0b1000
    // Branch on equal EL2
    beq     el2_to_el1

el3_to_el2:
    // Write 0101 1011 0001 (0x5B1) to SCR_EL3
    ldr     x1, =(SCR_VALUE)
    msr     scr_el3, x1
    // Write 0001 1100 1001 to SPSR EL3
    ldr     x2, =(SPSR_EL3_VALUE)
    msr     spsr_el3, x2
    // Load the address of the function el2_to_el1 and set the address to return from EL3 so that the call to etet jumps to this function to continue  
    adr     x3, el2_to_el1
    msr     elr_el3, x3
    eret

el2_to_el1:
    // Enable EL1 and EL0 to access the physical counter registers and physical timer registers  
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    // Enable EL1 physical timers
    msr     cnthctl_el2, x0
    // Clear virtual offset
    msr     cntvoff_el2, xzr
    // hcr_el2.rw = 1 (Register Width Control) Set to use aarch64 for EL1 (otherwise aarch32)
    // 0b0 Lower levels are all AArch32
    // 0b1 The Execution state for EL1 is AArch64. The Execution state forEL0 is determined by the current value of PSTATE.nRW when executing at EL0.
    // The Set/Way command is valid when Set at EL1
    mov     x1, #(1 << 31) 
    // hcr_el2.swio = 1 (Set/Way Invalidation Override)
    // 0b0 This control has no effect on the operation of data cache invalidate by set/way instructions.
    // 0b1 Data cache invalidate by set/way instructions perform a data cache clean and invalidate by set/way.
    orr     x1, x1, #(1 << 1) 
    msr     hcr_el2, x1
    mrs     x1, hcr_el2
    // Setup SCTLR access
    ldr     x3, =(SCTLR_VALUE_MMU_DISABLED_VALUE)
    msr     sctlr_el1, x3
    // Set SPSR_EL2
    ldr     x4, =(SPSR_EL2_VALUE)
    msr     spsr_el2, x4
    // Read the position of the symbol jump to main
    adr     x4, jump_to_main
    // Enter current EL2 state from "jump_to_main" so that EL1 state can jump to "jump_to_main" after using eret command  
    msr     elr_el2, x4
    eret

jump_to_main:
    // The CPU is in EL1 state
    // Config page table, gets the page table address
    adr     x2, kpgd 
    msr     ttbr0_el1, x2
    msr     ttbr1_el1, x2
    ldr     x3, =(TCR_VALUE)
    msr     tcr_el1, x3
    ldr     x4, =(MAIR_VALUE)
    msr     mair_el1, x4
    // Enable MMU
    mrs     x5, sctlr_el1
    orr     x5, x5, #SCTLR_MMU_ENABLED
    msr     sctlr_el1, x5 
    // Synchronize the instruction after the page table has been modified
    isb     
    //
    msr     spsel, #1
    mrs     x6, mpidr_el1
    and     x6, x6, #0xFF
    ldr     x7, =(STACK_SIZE)
    mul     x7, x7, x6
    ldr     x0, =_entry
    sub     x0, x0, x7
    mov     sp, x0
    ldr     x1, =main
    br      x1

proc_hang: 
    b       proc_hang