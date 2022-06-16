/**
 * @file uart.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef UART_H_
#define UART_H_

#include "peripherals_base.h"

#define AUX_ENABLES     (MMIO_BASE + 0x00215004)
#define AUX_MU_IO_REG   (MMIO_BASE + 0x00215040)
#define AUX_MU_IER_REG  (MMIO_BASE + 0x00215044)
#define AUX_MU_IIR_REG  (MMIO_BASE + 0x00215048)
#define AUX_MU_LCR_REG  (MMIO_BASE + 0x0021504C)
#define AUX_MU_MCR_REG  (MMIO_BASE + 0x00215050)
#define AUX_MU_LSR_REG  (MMIO_BASE + 0x00215054)
#define AUX_MU_MSR_REG  (MMIO_BASE + 0x00215058)
#define AUX_MU_SCRATCH  (MMIO_BASE + 0x0021505C)
#define AUX_MU_CNTL_REG (MMIO_BASE + 0x00215060)
#define AUX_MU_STAT_REG (MMIO_BASE + 0x00215064)
#define AUX_MU_BAUD_REG (MMIO_BASE + 0x00215068)

#define LSR_RX_READY    1
#define LSR_TX_IDLE     (1<<5)

#endif /* UART_H_ */