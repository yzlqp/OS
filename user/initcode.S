# Initial process execs /init.
# This code runs in user space

#include "../kernel/syscall/syscall.h"

# exec(init, argv)
.global start
start:
    ldr x0, =cmd
    ldr x1, =argv
    mov x8, #SYS_exec
    svc 0x0
    ldr x4, =start
    br x4

# char init[] = "/init\0";    
cmd:
    .string "/init\0"
argv:
    .long cmd
    .long 0
    .long 0
    .long 0
