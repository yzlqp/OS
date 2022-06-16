/**
 * @file bitops.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef BITOPS_H
#define BITOPS_H

#include "include/compiler_attributes.h"

/**
 * __ffsl - find first bit in word.
 * @word: The word to search
 *
 * Returns the lowest nonzero subscript of the input binary number
 * If 0 is passed, 0 is returned. 
 * Undefined if no bit exists, so code should check against 0 first.
 * __builtin_ctzl
 * This function returns the number of consecutive zeros in the input binary from the lowest digit (from the right).
 */
static __always_inline unsigned long __ffsl(unsigned long word)
{
	return __builtin_ctzl(word);
}

/**
 * __flsl - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs. Counting from 1
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static __always_inline int __flsl(unsigned long x)
{
	return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}

#endif /* BITOPS_H */