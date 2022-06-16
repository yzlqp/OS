/**
 * @file string.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "include/types.h"

void *memset(void *s, int c, size_t count)
{
	char *xs = s;
	while (count--)
		*xs++ = c;
	return s;
}

void *memcpy(void *dest, const void *src, size_t count)
{
	if (dest == NULL || src == NULL)
		return NULL;

	char *tmp = dest;
	const char *s = src;
	while (count--)
		*tmp++ = *s++;
	return dest;
}

void *memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;

	if (dest <= src) {
		tmp = dest;
		s = src;
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp = dest;
		tmp += count;
		s = src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}

int memcmp(const void *v1, const void *v2, size_t count)
{
	const uint8_t* s1 = (const uint8_t *)v1;
    const uint8_t* s2 = (const uint8_t *)v2;

    while (count-- > 0) {
        if (*s1 != *s2) 
			return (int)*s1 - (int)*s2;
        s1++, s2++;
    }

    return 0;
}

char *safestrcpy(char *s, const char *t, int n)
{
	char *os;

	os = s;
	if(n <= 0)
		return os;
	while(--n > 0 && (*s++ = *t++) != 0);
	*s = 0;
	return os;
}

char *strncpy(char *s, const char *t, int n)
{
	char *os;

	os = s;
	while(n-- > 0 && (*s++ = *t++) != 0);
	while(n-- > 0)
		*s++ = 0;
	return os;
}

int strncmp(const char *p, const char *q, unsigned int n)
{
	while(n > 0 && *p && *p == *q)
		n--, p++, q++;
	if(n == 0)
		return 0;
	return (unsigned char)*p - (unsigned char)*q;
}

int strlen(const char *s) 
{
	int n;
	for(n = 0; s[n]; n++);
	return n;
}