/**
 * @file trapframe.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TRAPFRAME_H
#define TRAPFRAME_H

#ifndef __ASSEMBLER__
#include <stdint.h>

struct trapframe {
    uint64_t regs[31];  // x0-x30 general purpose register
    uint64_t sp;        
    uint64_t pc;
    uint64_t pstate;
};

#define FRAME_SIZE sizeof(struct trapframe)
#else
#define FRAME_SIZE (34 * 8)
#endif

#endif /* TRAPFRAME_H */