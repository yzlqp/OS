/**
 * @file mian.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "console.h"
#include "printf.h"
#include "memory/kalloc.h"
#include "proc/proc.h"
#include "interrupt/interrupt.h"
#include "arch/aarch64/timer.h"
#include "file/file.h"
#include "buffer/buf.h"
#include "drivers/mmc/sd.h"
#include "lib/string.h"

extern void irq_init();
extern char edata[], edata_end[];

__attribute__((noreturn))
void main() 
{
    if (cpuid() == 0) {
        // Initialize .bss section data to zero
        memset(edata, 0, edata_end - edata);
        // Initialize console and UART
        console_init();
        // Initialize format print
        printf_init();
        cprintf("kernel booing...\n");
        
        // Initialize the memory management subsystem
        alloc_init();
        // Initialize the process management subsystem
        proc_init();
        // Initialize Interrupt exception subsystem, Load base address of EL1's exception vector table to vbar_el1
        exception_handler_init();
        // Initialize board level interrupt controller
        irq_init();
        // Initialize the Arm Generic Timer
        timer_init();
        // Enable all interrupt exceptions DAIF
        enable_interrupt();
        // Initialize the system buffer cache
        binit();
        // Initialize SD card and parse MBR
        sd_init();
        // initialize the inode table
        iinit();
        // initialize the kernel file table
        file_init();
        // Initialize the init process 
        init_user();
        // Wake up other cores
        init_awake_ap_by_spintable();
    } else {
        exception_handler_init();
        timer_init();
        enable_interrupt();
    }
    
    cprintf("main: [CPU %d] started.\n", cpuid());
    if (cpuid() == 0) {
        cprintf("\n------------------------\nKernel boot success.\n------------------------\n\n");
    }
    scheduler();
    while (1);
}