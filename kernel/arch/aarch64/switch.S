#include "include/context.h"

/*
 * The function prototype is
 * void swich(struct context *old, struct context *new);
 * The current context is saved on the stack, old is notified, and the given new context is loaded into the register
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/TPIDR-EL0--EL0-Read-Write-Software-Thread-ID-Register?lang=en
 */
.global swich
swich:
    // First save the current Callee-saved Register, The first argument is in x0
    stp     x19, x20, [x0, #16 * 0]
    stp     x21, x22, [x0, #16 * 1]
    stp     x23, x24, [x0, #16 * 2]
    stp     x25, x26, [x0, #16 * 3]
    stp     x27, x28, [x0, #16 * 4]
    stp     x29, x30, [x0, #16 * 5]
    mrs     x19, tpidr_el0
    mov     x20, sp
    stp     x19, x20, [x0, #16 * 6]
    // Get the given struct context *new
    // Because it's the second argument, *new is in x1
    // Load the new context into the register
    ldp     x23, x24, [x1, #16 * 6]
    msr     tpidr_el0, x23
    mov     sp, x24
    ldp     x19, x20, [x1, #16 * 0]
    ldp     x21, x22, [x1, #16 * 1]
    ldp     x23, x24, [x1, #16 * 2]
    ldp     x25, x26, [x1, #16 * 3]
    ldp     x27, x28, [x1, #16 * 4]
    ldp     x29, x30, [x1, #16 * 5]

    ret
