/**
 * @file timer.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TIMER_H
#define TIMER_H

#include "include/stdint.h"

/**
 * @brief  Initialize the Arm Generic Timer
 * @retval None
 */
void timer_init();

/**
 * @brief  If all goes well (irQ mask, etc.), set the next trigger time
 * @param  us: the next trigger time in us
 * @retval None
 */
void timer_tick_in(uint64_t us);

/**
 * @brief  Reset timer
 * @retval None
 */
void timer_reset();

#endif /* TIMER_H */

