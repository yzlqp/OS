/**
 * @file console.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include "include/stdint.h"
#include "arch/aarch64/board/raspi3/uart.h"
#include "sync/spinlock.h"
#include "proc/proc.h"
#include "include/param.h"
#include "file/file.h"

#define BACKSPACE 0x100
#define C(x) ((x) - '@')   // Control characters

extern struct devsw devsw[NDEV];
extern void uart_putchar(int c);
extern void uart_putchar_sync(int c);
extern void uart_init();

struct cons {
    struct spinlock lock;
    char buf[INPUT_BUF];
    uint32_t r;     // read index
    uint32_t w;     // write index
    uint32_t e;     // edit index
};
static struct cons cons;

/*
 * Sends a character to the console
 */
void console_putc(int c)
{
    if (c == BACKSPACE) {
        uart_putchar_sync('\b');
        uart_putchar_sync(' ');
        uart_putchar_sync('\b');
    } else {
        uart_putchar_sync(c);
    }
}

/*
 * Sends a character string to the console, return the number of characters written
 */
int console_write(char* src, int n)
{
    int i;
    for (i = 0; i < n; ++i) {
        uart_putchar(*(src + i));
    }
    return i;
}

/*
 * Read a character string to the console
 * copy (up to) a whole input line to dst.
 */
int console_read(char* dst, int n)
{
    int c;
    uint32_t target =  n;
    acquire_spin_lock(&cons.lock);
    while (n > 0) {
        // wait until interrupt handler has put some input into cons.buffer.
        while (cons.r == cons.w) {
            if (myproc()->killed) {
                release_spin_lock(&cons.lock);
                return -1;
            }
            sleep(&cons.r, &cons.lock);
        }
        c = cons.buf[cons.r++ % INPUT_BUF];
        if (c == C('D')) {
            // end of file
            if (n < target) {
                cons.r-- ;
            }
            break;
        }

        // copy the input byte to the user-space buffer.
        *dst = c;
        dst++;
        --n;
        if (c == '\n') { 
            // a whole line has arrived, return to the user-level read().
            break;
        }
    }
    release_spin_lock(&cons.lock);
    return target - n;
}

/*
 * the console input interrupt handler.
 * uartintr() calls this for input character.
 * do erase/kill processing, append to cons.buf,
 * wake up consoleread() if a whole line has arrived.
 */
void consoleintr(int c)
{
    acquire_spin_lock(&cons.lock);
    switch (c) {
        // Print process list.
        case C('P'):  
            // procdump
            break;

        // Kill line.
        case C('U'):
            while (cons.e != cons.w && cons.buf[(cons.e - 1) % INPUT_BUF] != '\n') {
                cons.e --;
                console_putc(BACKSPACE);
            }
            break;

        // Backspace
        case C('H'): 
        case '\x7f': 
            if (cons.e != cons.w) {
                cons.e -- ;
                console_putc(BACKSPACE);
            }
            break;
        default:
            if (c != 0 && cons.e - cons.r < INPUT_BUF) {
                c = (c == '\r') ? '\n' : c;

                // echo back to the user.
                console_putc(c);
                // store for consumption by consoleread().
                cons.buf[cons.e ++ % INPUT_BUF] = c;
                if (c == '\n' || c == C('D') || cons.e == cons.r + INPUT_BUF) {
                    // wake up consoleread() if a whole line (or end-of-file) has arrived.
                    cons.w = cons.e;
                    wakeup(&cons.r);
                }
            }
            break;
    }
    release_spin_lock(&cons.lock);
}

/*
 * Console initialization function
 */
void console_init(void)
{   
    // Initialize the console spinlock
    init_spin_lock(&cons.lock, "console");
    // Initialize the UART controller
    uart_init();
    // Configuring console device drivers
    devsw[CONSOLE].read = console_read;
    devsw[CONSOLE].write = console_write;
}
