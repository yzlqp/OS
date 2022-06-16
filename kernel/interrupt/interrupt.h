/**
 * @file interrupt.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdbool.h>

/**
 * @brief  Enable all interrupt exceptions DAIF (Interrupt Mask Bits)
 * @retval None
 */
void enable_interrupt();

/**
 * @brief  Mask all interrupt exceptions DAIF (Interrupt Mask Bits)
 * @retval None
 */
void disable_interrupt();

/**
 * @brief  Check whether all interrupt exceptions are enabled DAIF (Interrupt Mask Bits)
 * @retval The value of enable is 1, and that of disable is 0
 */
bool is_interrupt_enabled();

#endif /* INTERRUPT_H */