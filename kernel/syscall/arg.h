/**
 * @file arg.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ARG_H
#define ARG_H

#include "../include/stdint.h"

/**
 * @brief  The nth argument in the system call is used as a pointer and get the string
 * @param  n: parameter index
 * @param  pp: The address of a pointer to a string
 * @retval Returns the length of the string on success, and -1 on failure
 */
int64_t argstr(int n, char **pp);

/**
 * @brief  Assign the nth argument in the system call to PP as a pointer and check that the pointer is within the process valid bounds
 * @param  n: parameter index
 * @param  pp: The address of a pointer
 * @param  size: Pointer size
 * @retval 0 on success, and -1 on failure
 */
int64_t argptr(int n, char **pp, int size);

/**
 * @brief  Get the nth parameter in the system call. Up to 6 parameters are supported, that is, 0-5 parameters
 * @param  n: parameter index
 * @param  ip: Points to the value obtained
 * @retval Returns 0 on success, -1 on error
 */
int64_t argint(int n, uint64_t *ip);

/**
 * @brief  Fetch the nul-terminated string at addr from the current process.
 * @param  addr: Address of a string
 * @param  p: The address of a pointer to a string
 * @retval Returns the length of the string on success, and -1 on failure
 */
int64_t fetchstr(uint64_t addr, char **p);

/**
 * @brief  Get int64 value at user's virtual address ADDR, assign it to * IP and return -1 on failure
 * @param  addr: Address of a number
 * @param  *ip: Points to the value obtained
 * @retval Returns 0 on success, -1 on error
 */
int64_t fetchint64ataddr(uint64_t addr, uint64_t *ip);

#endif /* ARG_H */