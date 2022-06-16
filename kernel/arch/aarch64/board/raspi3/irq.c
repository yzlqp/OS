/**
 * @file irq.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "irq.h"
#include "../../timer.h"
#include "../../arm.h"

extern void enable_recv_interrupt(void);

/*
 * Board level interrupt controller initialization
 */
void irq_init()
{
    // AUX has an interrupt source for mini-UART
    put32(ENABLE_IRQS_1, AUX_INT);
    // Route all GPU interrupts to core 0
    put32(GPU_INTERRUPTS_ROUTING, GPUFIQ2CORE(0) | GPUIRQ2CORE(0));
    // Enable UART controller receive interrupt
    enable_recv_interrupt();
}