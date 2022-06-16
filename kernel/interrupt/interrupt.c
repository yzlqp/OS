/**
 * @file interrupt.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdbool.h>
#include "arch/aarch64/arm.h"
#include "interrupt.h"

extern void exception_handler_init(void);

/*
 * Enable all interrupt exceptions DAIF (Interrupt Mask Bits)
 */
void enable_interrupt()
{
    clear_daif();
}

/*
 * Mask all interrupt exceptions DAIF (Interrupt Mask Bits)
 */
void disable_interrupt()
{
    set_daif();
}

/*
 * Check whether all interrupt exceptions are enabled DAIF (Interrupt Mask Bits)
 */
bool is_interrupt_enabled()
{
    return get_daif() == 0;
}