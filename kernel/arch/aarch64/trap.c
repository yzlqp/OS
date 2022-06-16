/**
 * @file trap.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdint.h>
#include "board/raspi3/local_peripherals.h"
#include "board/raspi3/irq.h"
#include "include/exception.h"
#include "include/trapframe.h"
#include "arm.h"
#include "timer.h"
#include "printf.h"
#include "proc/proc.h"
#include "board/raspi3/uart.h"

extern int64_t syscall(struct trapframe *frame);
extern void uartintr(void);
extern void clock_intr(void);

/*
 * Unused exceptions, used for debugging
 */
void trap_error(uint64_t error_type)
{
    switch (error_type) {
        case(BAD_SYNC_SP0):
            panic("BAD_SYNC_SP0");
            break;
        case(BAD_IRQ_SP0):
            panic("BAD_IRQ_SP0");
            break;
        case(BAD_FIQ_SP0):
            panic("BAD_FIQ_SP0");
            break; 
        case(BAD_ERROR_SP0):
            panic("BAD_ERROR_SP0");
            break;
        case(BAD_FIQ_SPx):
            panic("BAD_FIQ_SPx");
            break;
        case(BAD_ERROR_SPx):
            panic("BAD_ERROR_SPx");
            break;
        case(BAD_FIQ_EL0):
            panic("BAD_FIQ_EL0");
            break;
        case(BAD_ERROR_EL0):
            panic("BAD_ERROR_EL0");
            break;
        case(BAD_AARCH32):
            panic("BAD_AARCH32");
            break;
        default:
            panic("Unknown exception");
    }
}

/*
 * Synchronous Exception Handling, from EL1
 */
void el_sync_trap(struct trapframe *frame_ptr, uint64_t esr)
{
    uint64_t exception_class_id = (esr >> 26) & 0b111111;
    uint64_t iss = (esr & ISS_MASK);
    panic("el_sync_trap: exception class id: 0x%x, iss: %d", exception_class_id, iss);
}

/*
 * Synchronous Exception Handling, from EL0
 */
void el0_sync_trap(struct trapframe *frame, uint64_t esr)
{
    //cprintf("enter el0_sync_trap.\n");
    uint64_t exception_class_id = (esr >> 26) & 0b111111;
    uint64_t iss = (esr & ISS_MASK);
    if (exception_class_id == EC_SVC64 && iss == 0) {
        struct proc *p = myproc();
        if (p->killed) {
            exit(-1);
        }
        p->tf = frame;
        //enable_interrupt();
        p->tf->regs[0] = syscall(frame);
        
        if (p->killed) {
            exit(-1);
        }
    } else {
        panic("el0_sync_trap: exception class id: 0x%x, iss: %d", exception_class_id, iss);
    }
}

/*
 * To obtain an irq source 
 */
uint32_t read_irq_src()
{
    return get32(COREn_IRQ_SOURCE(cpuid()));
}

/*
 * Irq handlers (architecture dependent)
 */
void handle_arch_irq(struct trapframe *frame_ptr)
{
    uint32_t irq_src = read_irq_src();
    // If the current core has a time interrupt
    if (irq_src & IRQ_CNTPNSIRQ) {
        if (cpuid() == 0) {
            clock_intr();
        }
        timer_reset();
        yield();
    } else {
        uint32_t irq_pending_1 = get32(IRQ_PENDING_1);
        uint32_t irq_pending_2 = get32(IRQ_PENDING_2);
        if (irq_pending_1 & AUX_INT) {
            uartintr();
        }
    }
}

/*
 * Interrupt subsystem initialization, Load base address of EL1's exception vector table to vbar_el1
 */
void exception_handler_init(void)
{
    extern char vectors[];
    load_vbar_el1(vectors);
    isb();
}
