/**
 * @file local_peripherals.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef LOCAL_PERIPHERALS_H
#define LOCAL_PERIPHERALS_H

#include "peripherals_base.h"

/*
 * Bcm2837 has the same definition as BCM2836.
 * So physical memory is in the range of 0x4000 0000-0x4001 FFFF 
 * It can be used to control ARM Timer, IRQ and Mailbox 
 */

#define CONTROL_REGISTER        (LOCAL_PERIPHERALS_BASE + 0)
#define CORE_TIMER_PRESCALER    (LOCAL_PERIPHERALS_BASE + 0x8)
#define GPUFIQ2CORE(i)          (((i) & 0x11) << 2)
#define GPUIRQ2CORE(i)          ((i) & 0x11)

#define GPU_INTERRUPTS_ROUTING                       (LOCAL_PERIPHERALS_BASE + 0xC)
#define PERFORMANCE_MONITOR_INTERRUPT_ROUTINT_SET    (LOCAL_PERIPHERALS_BASE + 0x10)
#define PERFORMANCE_MONITOR_INTERRUPT_ROUTINT_CLEAR  (LOCAL_PERIPHERALS_BASE + 0x14)
#define CORE_TIMER_ACCESS_LS                         (LOCAL_PERIPHERALS_BASE + 0x1C)
#define CORE_TIMER_ACCESS_MS                         (LOCAL_PERIPHERALS_BASE + 0x20)
#define LOCAL_INTERRUPTS_ROUTING                     (LOCAL_PERIPHERALS_BASE + 0x24)
#define AXI_OUTSTANDING_COUNTERS                     (LOCAL_PERIPHERALS_BASE + 0x2C)
#define AXI_OUTSTANDING_IRQ                          (LOCAL_PERIPHERALS_BASE + 0x30)
#define LOCAL_TIMER_CONTROL_AND_STATUS               (LOCAL_PERIPHERALS_BASE + 0x34)
#define LOCAL_TIMER_WRITE_FLAGS                      (LOCAL_PERIPHERALS_BASE + 0x38)
#define COREn_TIMER_INTERRUPT_CONTROL(i)             (LOCAL_PERIPHERALS_BASE + 0x40 + (i) * 4)
#define COREn_MAILBOX_INTERRUPT_CONTROL(i)           (LOCAL_PERIPHERALS_BASE + 0x50 + (i) * 4)
#define COREn_IRQ_SOURCE(i)                          (LOCAL_PERIPHERALS_BASE + 0x60 + (i) * 4)
#define COREn_FIQ_SOURCE(i)                          (LOCAL_PERIPHERALS_BASE + 0x70 + (i) * 4)
#define COREx_MAILBOXy_WRITE_SET(x, y)               (LOCAL_PERIPHERALS_BASE + 0x80 + (x) * 0x10 + (y) * 4)
#define COREx_MAILBOXy_READ_WRITE_HIGH_TO_CLEAR(x,y) (LOCAL_PERIPHERALS_BASE + 0xC0 + (x) * 0x10 + (y) * 4)

// Core Timer (Generic Timer)  Use only CNTPNSIRQ (non-secure physical timer)
#define CORE_TIMER_ENABLE (1 << 1) 

#endif /* LOCAL_PERIPHERALS */