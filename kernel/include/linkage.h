/**
 * @file linkage.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef LINKAGE_H
#define LINKAGE_H

#ifndef ASM_NL
#define ASM_NL		 ;
#endif

#ifdef __ASSEMBLER__

    #ifndef ENTRY
    #define ENTRY(name) ASM_NL \
        .global name ASM_NL\
        name:
    #endif

    #ifndef WEAK
    #define WEAK(name) \
        .weak name ASM_NL \
        name:
    #endif

    #ifndef END
    #define END(name) \
        .size name, .-name
    #endif

    #ifndef ENDPROC
    #define ENDPROC(name) \
        .type name, @function ASM_NL \
        END(name)
    #endif

#endif

#endif /* LINKAGE_H */