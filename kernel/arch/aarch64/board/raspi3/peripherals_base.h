/**
 * @file peripherals_base.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef PERIPHERALS_BASE_
#define PERIPHERALS_BASE_

#include "memlayout.h"

#ifndef KERNEL_BASE
#define KERNEL_BASE 0x0
#endif

#define MMIO_BASE               (KERNEL_BASE + 0x3F000000)
#define LOCAL_PERIPHERALS_BASE  (KERNEL_BASE + 0x40000000)

#endif /* PERIPHERALS_BASE_ */