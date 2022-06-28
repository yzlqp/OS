/**
 * @file printf.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include <stdint.h>

/*
 *  Assertion macro
 */
#define assert(x)                                                              \
    ({                                                                         \
        if (!(x)) {                                                            \
            panic("%s:%d: assertion failed.\n", __FILE__, __LINE__);           \
        }                                                                      \
    })

/*
 *  Assertion with reason
 */
#define asserts(x, ...)                                                        \
    ({                                                                         \
        if (!(x)) {                                                            \
            cprintf("%s:%d: assertion failed.\n", __FILE__, __LINE__);         \
            panic(__VA_ARGS__);                                                \
        }                                                                      \
    })

/**
 * @brief  Format print initialization
 * @retval None
 */
void printf_init();

/**
 * @brief  Kernel panic printing function
 * @param  fmt Format string
 * @param  ... Variable parameters
 * @retval None
 */
void panic(const char *fmt, ...);

/**
 * @brief  Generic format print function 
 * @param  fmt Format string
 * @param  ... Variable parameters
 * @retval None
 */
void cprintf(const char *fmt, ...);

#endif /* PRINTF_H */