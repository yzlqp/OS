/**
 * @file console.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <stdarg.h>
#include <stdint.h>

/**
 * @brief  Console initialization function
 * @retval None
 */
void console_init(void);

/**
 * @brief  Sends a character to the console
 * @param  c: The character to send
 * @retval None
 */
void console_putc(int c);

/**
 * @brief  Sends a character string to the console
 * @param  src: The first address of character string to send
 * @param  n: The number of characters to send
 * @retval returns the number of characters has been sent
 */
int console_write(char* src, int n);

/**
 * @brief  Read a character string to the console
 * @param  src: The first address of character string to read
 * @param  n: The number of characters to read
 * @retval returns the number of characters has been read
 */
int console_read(char* dst, int n);

/**
 * @brief  the console input interrupt handler.
 * @param  c: The character to send
 * @retval None
 */
void consoleintr(int c);

#endif /* _CONSOLE_H_ */