/**
 * @file printf.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdarg.h>
#include <stdbool.h>
#include "include/stdint.h"
#include "sync/spinlock.h"
#include "console.h"

void panic(const char *fmt, ...);

volatile int panicked = 0;
static char digits[] = "0123456789ABCDEF";
static struct print_lock {
    struct spinlock lock;
    bool locking;
} print_lock;

/*
 * Prints int numbers to the serial port, param x represent The value of the digital
 * param base represent Digital base, param sign represent Whether a number has a sign bit
 */
static void printint(int64_t x, int base, int sign)
{
    static char buf[64];
    if (sign && (sign = x < 0)) {
        x = -x;
    }
    int i = 0;
    uint64_t t = x;
    do {
        buf[i++] = digits[t % base];
    } while ((t /= base) != 0);
    if (sign) {
        buf[i++] = '-';
    }
    // Reverse printing
    while (i-- >= 0) {
        console_putc(buf[i]);
    }
}

/* 
 * Prints the value of the pointer in hexadecimal format, for example, 0x0000 0000 0000 FFFF  
 */
static void printptr(uint64_t x)
{
    console_putc('0');
    console_putc('x');
    for (int i = 0; i < (sizeof(uint64_t) * 2); ++i, x <<= 4) {
        console_putc(digits[x >> (sizeof(uint64_t) * 8 - 4)]);
    }
}

/*
 * Format print function, param putch represent Print character hook function
 * param fmt represent Format string, param ap represent The argument list of a variable argument function
 */
static void vprintfmt(void (*putch)(int), const char *fmt, va_list ap)
{
    int i, c;
    char *s;
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        // Handles the % format control symbol
        if (c != '%') {
            putch(c);
            continue;
        }
        // Count the number of l after the % symbol
        int l = 0;
        for (; fmt[i+1] == 'l'; i++)
            l++;
        // Handling special cases, make sure there are unprocessed characters after it
        if (!(c = fmt[++i] & 0xff))
            break;

        switch (c) {
        case 'u':
            if (l == 2) 
                printint(va_arg(ap, int64_t), 10, 0);
            else 
                printint(va_arg(ap, int), 10, 0);
            break;
        case 'd':
            if (l == 2) 
                printint(va_arg(ap, int64_t), 10, 1);
            else 
                printint(va_arg(ap, int), 10, 1);
            break;
        case 'x':
            if (l == 2) 
                printint(va_arg(ap, int64_t), 16, 0);
            else 
                printint(va_arg(ap, int), 16, 0);
            break;
        case 'p':
            printptr((int64_t)va_arg(ap, void *));
            break;
        case 'c':
            putch(va_arg(ap, int));
            break;
        case 's':
            if ((s = (char*)va_arg(ap, char *)) == 0)
                s = "(null)";
            for (; *s; s++)
                putch(*s);
            break;
        case '%':
            putch('%');
            break;
        default:
            // Print unknown % sequence to draw attention
            putch('%');
            putch(c);
            break;
        }
    }
}

/*
 * Generic format print function
 */
__attribute__((format(printf, 1, 2)))
void cprintf(const char *fmt, ...)
{
    bool locking = print_lock.locking;
    if (locking)
        acquire_spin_lock(&print_lock.lock);
    if (fmt == NULL) {
        panic("%s: fmt is NULL", __FUNCTION__);
    }
    va_list ap;
    va_start(ap, fmt);
    vprintfmt(console_putc, fmt, ap);
    va_end(ap);
    if (locking) {
        release_spin_lock(&print_lock.lock);
    }
}

/*
 * Kernel panic printing function
 */
__attribute__((format(printf, 1, 2)))
__attribute__((noreturn))
void panic(const char *fmt, ...)
{
    print_lock.locking = false;
    va_list ap;
    va_start(ap, fmt);
    vprintfmt(console_putc, fmt, ap);
    va_end(ap);
    cprintf("\t%s:%d: kernel panic.\n", __FILE__, __LINE__);
    panicked = true;
    while(1);  
}

/*
 * Format print initialization
 */
void printf_init(void)
{
    // Initialize the printf spinlock
    init_spin_lock(&print_lock.lock, "pr");
    print_lock.locking = 1;
    cprintf("********************************************************************************\n");
}

