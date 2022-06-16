/**
 * @file string.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef STRING_H
#define STRING_H

#include "include/types.h"

void *memset(void *s, int c, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *s1, const void *s2, size_t count);
char *safestrcpy(char *s, const char *t, int n);
char *strncpy(char *s, const char *t, int n);
int strncmp(const char *p, const char *q, unsigned int n);
int strlen(const char *s);

#endif /* STRING_H */