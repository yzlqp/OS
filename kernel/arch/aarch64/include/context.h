/**
 * @file context.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#ifndef __ASSEMBLER__
#include "../../../include/stdint.h"

/*
 * The register to be saved by the caller when a context switch is performed
 */
struct context {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x29;       // Stack Frame Pointer register
    uint64_t x30;       // Link register (the address to return)
    uint64_t tpidr_el0; // "Thread ID" Register
    uint64_t sp_el1;    // Stack Pointer register
};

#define CONTEXT_SIZE sizeof(struct context)
#else
#define CONTEXT_SIZE (12 * 8)
#endif

#endif /* CONTEXT_H */