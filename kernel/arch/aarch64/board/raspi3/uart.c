/**
 * @file uart.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdarg.h>
#include "../../../../include/stdint.h"
#include "../../../../proc/proc.h"
#include "uart.h"
#include "gpio.h"
#include "../../arm.h"
#include "sync/spinlock.h"

#define UART_TXBUF_SIZE 32

char uart_tx_buf[UART_TXBUF_SIZE];
uint64_t uart_tx_w;     // Next write location, uart_tx_buf[uart_tx_w % UART_TXBUF_SIZE]  
uint64_t uart_tx_r;     // Next read location, uart_tx_buf[uart_tx_r % UART_TXBUF_SIZE]
extern volatile int panicked;
extern void consoleintr(int c);
static struct spinlock uart_tx_lock;

/*
 * Check whether the UART controller send buffer is empty
 */
static inline int is_send_buffer_empty()
{
    return (get32(AUX_MU_IIR_REG) & (1 << 1)) != 0;
}

/*
 * Check whether the UART controller receive buffer is non-empty
 */
static inline int is_recv_buffer_not_empty()
{
    return (get32(AUX_MU_IIR_REG) & (1 << 2)) != 0;
}

/*
 * UART controller, enable receive interrupt
 */
void enable_recv_interrupt(void)
{
    uint32_t value = get32(AUX_MU_IER_REG);
    value |= 0b1;
    put32(AUX_MU_IER_REG, value);
}

/*
 * UART controller, disable receive interrupt
 */
void disable_recv_interrupt(void)
{
    uint32_t value = get32(AUX_MU_IER_REG);
    value &= ~(0b1);
    put32(AUX_MU_IER_REG, value);
}

/*
 * UART controller serial port initialization function 
 */
void uart_init(void)
{
    uint32_t selector;
    selector = get32(GPFSEL1);
    selector &= ~(7 << 12);         // Clean gpio14
    selector |= 2 << 12;            // Set alt5 for gpio14 
    selector &= ~(7 << 15);         // Clean gpio15 
    selector |= 2 << 15;            // Set alt5 for gpio15 
    put32(GPFSEL1, selector);

    put32(GPPUD, 0);
    delay(150);
    put32(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    put32(GPPUDCLK0, 0);

    put32(AUX_ENABLES, 1);          // Enable mini_uart
    put32(AUX_MU_CNTL_REG, 0);      // The flow control function of the serial port is disabled
    put32(AUX_MU_IER_REG, 0);       // Turn off the serial port transmission interruption, then configure the serial port
    put32(AUX_MU_LCR_REG, 3);       // Set the serial port to the 8bit mode
    put32(AUX_MU_MCR_REG, 0);       // Set RTS to always high
    put32(AUX_MU_BAUD_REG, 270);    // Set the baud rate to 115200
    put32(AUX_MU_IIR_REG, ((0b11 << 1) | (0b11 << 6))); // Clear the FIFO for receiving and sending and enable fifo
    put32(AUX_MU_CNTL_REG, 3);      // Re-enable serial port transmission
    //put32(AUX_MU_IER_REG, 0b11);  // Open the serial port send and receive interrupt 
}

/*
 * if the UART is idle, and a character is waiting in the transmit buffer, send it.
 * caller must hold uart_tx_lock. called from both the top- and bottom-half.
 */
static void uart_start()
{
    while(1) {
        // If the queue is empty, exit
        if (uart_tx_w == uart_tx_r) 
            return;

        // If the UART module is busy at this time, it will not be able to transfer
        if ((get32(AUX_MU_LSR_REG) & LSR_TX_IDLE) == 0)
            return; 
    
        // Gets the next character to write
        int c = uart_tx_buf[uart_tx_r % UART_TXBUF_SIZE];
        uart_tx_r += 1;
        // maybe uartputc() is waiting for space in the buffer.
        wakeup(&uart_tx_r);
        // Writes data to controller registers
        put32(AUX_MU_IO_REG, c);
    }
}

/*
 * Sends a character asynchronously to the serial port
 */
void uart_putchar(int c)
{
    acquire_spin_lock(&uart_tx_lock);
    if (panicked) {
        while(1);
    }
    while(1) {
        // The buffer is full, wait for uartstart() to open up space in the buffer.
        if (uart_tx_w == uart_tx_r + UART_TXBUF_SIZE) {
            sleep(&uart_tx_r, &uart_tx_lock);
        } else {
            uart_tx_buf[uart_tx_w % UART_TXBUF_SIZE] = c;
            uart_tx_w += 1;
            uart_start();
            release_spin_lock(&uart_tx_lock);
            return;
        }
    }
}

/*
 * Sends a character to the serial port, synchronization
 */
void uart_putchar_sync(int c)
{
    push_off();
    if (panicked) { 
        while(1);
    }
    // Wait for the send queue to be available
    while((get32(AUX_MU_LSR_REG) & LSR_TX_IDLE) == 0);
    put32(AUX_MU_IO_REG, c);
    pop_off();
}

/*
 * read one input character from the UART. return -1 if none is waiting.
 */
static int uart_getchar(void)
{
    // If there is data then read，otherwise return failure
    if (get32(AUX_MU_LSR_REG) & LSR_RX_READY) {
        return get32(AUX_MU_IO_REG);
    } else {
        return -1;
    }
}

/*
 * handle a uart interrupt, raised because input has arrived, or the 
 * uart is ready for more output, or both. called from trap.c.
 * Print UART controller read buffer data to console and 
 * write all of user write buffer data to controller write buffer reg 
 */
void uartintr(void)
{
    // read and process incoming characters.
    while(1) {
        int c = uart_getchar();
        if (c == -1) {
            break;
        }
        consoleintr(c);
    }

    // send buffered characters.
    acquire_spin_lock(&uart_tx_lock);
    uart_start();
    release_spin_lock(&uart_tx_lock); 
}

