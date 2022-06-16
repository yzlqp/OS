/**
 * @file compiler_attributes.h
 * @author ylp
 * @brief Refer to the Linux kernel source code include/linux/compiler_attributes.h (5.10.0)
 * @version 0.1
 * @date 2022-01-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef COMPILER_ATTRIBUTES_H
#define COMPILER_ATTRIBUTES_H

#include <stddef.h>

/*
 * Note the missing underscores.
 *
 * gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-noinline-function-attribute
 * clang: mentioned
 */
#define   noinline                      __attribute__((__noinline__))

/*
 * Note: users of __always_inline currently do not write "inline" themselves,
 * which seems to be required by gcc to apply the attribute according
 * to its docs (and also "warning: always_inline function might not be
 * inlinable [-Wattributes]" is emitted).
 *
 * gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-always_005finline-function-attribute
 * clang: mentioned
 */
#define __always_inline                 inline __attribute__((__always_inline__))


#ifndef barrier
/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")
#endif

typedef uint8_t  __attribute__((__may_alias__)) __u8_alias_t;
typedef uint16_t __attribute__((__may_alias__)) __u16_alias_t;
typedef uint32_t __attribute__((__may_alias__)) __u32_alias_t;
typedef uint64_t __attribute__((__may_alias__)) __u64_alias_t;

static __always_inline void __read_once_size(const volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(__u8_alias_t  *) res = *(volatile __u8_alias_t  *) p; break;
	case 2: *(__u16_alias_t *) res = *(volatile __u16_alias_t *) p; break;
	case 4: *(__u32_alias_t *) res = *(volatile __u32_alias_t *) p; break;
	case 8: *(__u64_alias_t *) res = *(volatile __u64_alias_t *) p; break;
	default:
		barrier();
		__builtin_memcpy((void *)res, (const void *)p, size);
		barrier();
	}
}

static __always_inline void __write_once_size(volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(volatile  __u8_alias_t *) p = *(__u8_alias_t  *) res; break;
	case 2: *(volatile __u16_alias_t *) p = *(__u16_alias_t *) res; break;
	case 4: *(volatile __u32_alias_t *) p = *(__u32_alias_t *) res; break;
	case 8: *(volatile __u64_alias_t *) p = *(__u64_alias_t *) res; break;
	default:
		barrier();
		__builtin_memcpy((void *)p, (const void *)res, size);
		barrier();
	}
}

#define READ_ONCE(x)								\
({													\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__c = { 0 } };							\
	__read_once_size(&(x), __u.__c, sizeof(x));		\
	__u.__val;										\
})

#define WRITE_ONCE(x, val)							\
({													\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__val = (val) }; 						\
	__write_once_size(&(x), __u.__c, sizeof(x));	\
	__u.__val;										\
})

#if __has_attribute(__error__)
#define __compiletime_error(msg)       __attribute__((__error__(msg)))
#else
#define __compiletime_error(msg)
#endif

#endif /* COMPILER_ATTRIBUTES_H */