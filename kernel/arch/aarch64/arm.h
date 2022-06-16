/**
 * @file arm.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _ARM_H
#define _ARM_H

#include <stdint.h>
typedef uint64_t   pte_t;
typedef uint64_t * pagetable_t;

/*
 * Writing a 32-bit value to the specified memory location controls the compiler not to optimize the write
 */
static inline void put32(uint64_t ptr, uint32_t val)
{
    *(volatile uint32_t *)ptr = val;
}

/*
 * Reading directly from the specified memory location will control the compiler not to optimize
 */
static inline uint32_t get32(uint64_t ptr)
{
    return *(volatile uint32_t *)ptr;
}

/*
 * Read Exception Syndrome Register (EL1)
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-?lang=en
 */
static inline uint64_t read_esr_el1()
{
    uint64_t reg;
    asm volatile (
        "mrs %0, esr_el1"
        : "=r" (reg)
    );
    return reg;
}

/*
 * Read Exception Link Register (EL1)
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/ELR-EL1--Exception-Link-Register--EL1-?lang=en
 */
static inline uint64_t read_elr_el()
{
    uint64_t reg;
    asm volatile (
        "mrs %0, elr_el1"
        : "=r" (reg)
    );
    return reg;
}

/*
 * Instruction synchronization barrier (strictest) 
 * It cleans the pipeline retrieve instructions and data execution, and ensure that all the instructions in front of it have been executed before executing the instructions that follow it.
 */
static inline void isb()
{
    asm volatile (
        "isb"
    );
}

/*
 * Data synchronization Barrier
 * All barrier instructions can take field specifiers, and all unsupported field specifiers are considered system fields
 * DMB can continue to execute subsequent instructions as long as the instruction is not a memory access instruction. 
 * But the DSB forces the CPU to wait for whatever instructions follow it to complete.
 * The ISB not only does what the DSB does, it also empties the pipeline
 */
static inline void disb()
{
    asm volatile("dsb sy; isb");
}

/*
 * SCTLR_EL1 bit[26] UCI
 * When this is set, DC CVAU, DC CIVAC, DC CVAC and IC IVAU instructions can be accessed in EL0 of AArch64.
 * DC CIVAC Data cache clean and invalidate by VA to PoC
 * https://developer.arm.com/documentation/ddi0488/h/system-control/aarch64-register-summary/aarch64-cache-maintenance-operations?lang=en
 */
static inline void dccivac(void *p, int n)
{
    while (n--) 
        asm volatile("dc civac, %[x]" : : [x] "r"(p + n));
}

/*
 * CNTPCT, Counter-timer Physical Count register
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch32-Registers/CNTPCT--Counter-timer-Physical-Count-register?lang=en
 */
static inline uint64_t timestamp()
{
    uint64_t t;
    asm volatile (
        "mrs %[cnt], cntpct_el0" 
        : [cnt] "=r"(t)
    );
    return t;
}

/*
 * Sets the base address of EL1's exception handler table
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/VBAR-EL1--Vector-Base-Address-Register--EL1-?lang=en
 */
static inline void load_vbar_el1(void *ptr)
{
    asm volatile (
        "msr vbar_el1, %0"
        : /* no output */
        : "r" (ptr)
    );
}

/*
 * Set the stack pointer to EL1
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/SP-EL1--Stack-Pointer--EL1-?lang=en
 */
static inline void load_sp_el1(void *ptr)
{
    asm volatile (
        "msr sp_el1, %0"
        : /* no output */
        : "r" (ptr)
    );
}

/*
 * A delay is used to wait for the serial port, but the count is not exact, just a loop
 */
static inline void delay(int32_t count)
{
    asm volatile (
        "__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
        :"=r"(count)
        :[count]"0"(count)
        :"cc"
    );
}
/*
 * MPIDR_EL1, Multiprocessor Affinity Register
 * In a multiprocessor system, provides an additional PE identification mechanism for scheduling purposes.
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch32-Registers/MPIDR--Multiprocessor-Affinity-Register?lang=en
 */
static inline uint64_t r_mpidr()
{
    uint64_t value;
    asm volatile (
        "mrs %0, mpidr_el1"
        : "=r"(value)
    );
    return value;
}

/*
 * Read clock frequency
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch32-Registers/CNTFRQ--Counter-timer-Frequency-register?lang=en
 */
static inline uint64_t r_cntfrq_el0()
{
    uint64_t value;
    asm volatile (
        "mrs %0, cntfrq_el0"
        : "=r" (value)
    );
    return value;
}

/*
 * Read Counter-timer Physical Timer Control register
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/CNTP-CTL-EL0--Counter-timer-Physical-Timer-Control-register?lang=en
 */
static inline uint64_t r_cntp_ctl_el0()
{
    uint64_t value;
    asm volatile (
        "mrs %0, cntp_ctl_el0"
        : "=r"(value)
    );
    return value;
}

/*
 * load and set Counter-timer Physical Timer Control register
 */
static inline void l_cntp_ctl_el0(uint64_t value)
{
    asm volatile (
        "msr cntp_ctl_el0, %0"
        : /* no output */
        :"r"(value)
    );
}

/*
 * Read Counter-timer Physical Timer TimerValue register
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/CNTP-TVAL-EL0--Counter-timer-Physical-Timer-TimerValue-register?lang=en
 */
static inline uint64_t r_cntp_tval_el0()
{
    uint64_t value;
    asm volatile (
        "mrs %0, cntp_tval_el0"
        : "=r"(value)
    );
    return value;
}

/*
 * Load and set Counter-timer Physical Timer TimerValue register
 */
static inline void l_cntp_tval_el0(uint64_t value)
{
    asm volatile (
        "msr cntp_tval_el0, %0"
        : /* no output */
        :"r"(value)
    );
}

/*
 * clear DAIF, Interrupt Mask Bits
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/DAIF--Interrupt-Mask-Bits?lang=en
 */
static inline void clear_daif()
{
    asm volatile (
        "msr daif, %[x]" 
        : /* no output */
        : [x] "r"(0)
    );
}

/*
 * set all bit to DAIF, Interrupt Mask Bits
 * D [9]: debug exception MASK (Watchpoint, Breakpoint and Software Step exceptions)
 * A [8]: SError interrupt MASK
 * I [7]：IRQ interrupt MASK
 * F [6]：FIQ interrupt MASK
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/DAIF--Interrupt-Mask-Bits?lang=en
 */
static inline void set_daif()
{
    asm volatile (
        "msr daif, %[x]" 
        : /* no output */
        : [x] "r"(0xF << 6)
    );
}

/*
 * get DAIF, Interrupt Mask Bits
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/DAIF--Interrupt-Mask-Bits?lang=en
 * TODO
 */
static inline uint32_t get_daif()
{
    uint32_t value;
    asm volatile (
        "mrs %0, daif"
        : "=r" (value)
    );
    return value;
}

/*
 * Us level delay, synchronization, non-sleep
 * CNTPCT_EL0, Counter-timer Physical Count register
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/CNTPCT-EL0--Counter-timer-Physical-Count-register?lang=en
 */
static inline void delayus(uint32_t n)  // TODO
{
    uint64_t f,t,r;
    // First, get the current counter frequency
    asm volatile("mrs %[freq], cntfrq_el0" : [freq] "=r"(f));
    // Gets the value of the current counter
    asm volatile("mrs %[cnt], cntpct_el0" : [cnt] "=r"(t));
    
    t += f / 1000000 * n;
    do {
        asm volatile("mrs %[cnt], cntpct_el0" : [cnt] "=r"(r));
    } while (r < t);
}

/*
 * Load Translation Table Base Register 0 (EL1)
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/TTBR0-EL1--Translation-Table-Base-Register-0--EL1-?lang=en
 */
static inline void lttbr0(uint64_t p) 
{
    asm volatile("msr ttbr0_el1, %[x]" : : [x] "r"(p));
    disb();
    asm volatile("tlbi vmalle1");
    disb();
}

/*
 * Load Translation Table Base Register 1 (EL1)
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/TTBR1-EL1--Translation-Table-Base-Register-1--EL1-?lang=en
 */
static inline void lttbr1(uint64_t p)
{
    asm volatile("msr ttbr1_el1, %[x]" : : [x] "r"(p));
    disb();
    asm volatile("tlbi vmalle1");
    disb();
}

#endif /* _ARM_H */