/**
 * @file printf.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-05-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "user.h"
#include <stdarg.h>

static char digits[] = "0123456789ABCDEF";

static void putc(int fd, char c)
{
	write(fd, &c, 1);
}

/*
 * Prints the value of the number
 */
static void printint(int fd, int xx, int base, int sgn)
{
	char buf[16];
	int i, neg;
	uint x;

	neg = 0;
	if (sgn && xx < 0) {
		neg = 1;
		x = -xx;
	} else {
		x = xx;
	}

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while ((x /= base) != 0);
	if (neg)
		buf[i++] = '-';

	while (--i >= 0)
		putc(fd, buf[i]);
}

/*
 * Prints the value of the pointer
 */
static void printptr(int fd, uint64 x) 
{
	int i;
	putc(fd, '0');
	putc(fd, 'x');
	for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
		putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
}

/*
 * Print to the given fd. Only understands %d, %x, %p, %s.
 */
void vprintf(int fd, const char *fmt, va_list ap)
{
	char *s;
	int c, i, state;

	state = 0;
	for (i = 0; fmt[i]; i++) {
		c = fmt[i] & 0xff;
		if (state == 0) {
			if(c == '%') {
				state = '%';
			} else {
				putc(fd, c);
			}
		} else if (state == '%') {
			if (c == 'd') {
				printint(fd, va_arg(ap, int), 10, 1);
			} else if (c == 'l') {
				printint(fd, va_arg(ap, uint64), 10, 0);
			} else if (c == 'x') {
				printint(fd, va_arg(ap, int), 16, 0);
			} else if (c == 'p') {
				printptr(fd, va_arg(ap, uint64));
			} else if (c == 's') {
				s = va_arg(ap, char*);
				if(s == 0)
					s = "(null)";
				while(*s != 0) {
					putc(fd, *s);
					s++;
				}
			} else if (c == 'c') {
				putc(fd, va_arg(ap, uint));
			} else if (c == '%') {
				putc(fd, c);
			} else {
				// Unknown % sequence. Print it to draw attention.
				putc(fd, '%');
				putc(fd, c);
			}
			state = 0;
		}
	}
}

/*
 * Format print to the specified fd
 */
void fprintf(int fd, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fd, fmt, ap);
	va_end(ap);
}

/*
 * Format print to standard output (fd = 1)
 */
void printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(1, fmt, ap);
	va_end(ap);
}
