/**
 * @file exception.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

#define BAD_SYNC_SP0    0
#define BAD_IRQ_SP0     1
#define BAD_FIQ_SP0     2
#define BAD_ERROR_SP0   3

#define BAD_FIQ_SPx     4
#define BAD_ERROR_SPx   5

#define BAD_FIQ_EL0     6
#define BAD_ERROR_EL0   7

#define BAD_AARCH32     8

/* 
 * EC [31:26] in ESR_EL1(Exception Syndrome Register) 
 * Exception Class. Indicates the reason for the exception that this register holds information about.
 * https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-?lang=en
 */
#define EC_Unknown                  0x0         // Unknown reason.
#define EC_WF_INSTRUCTION           0x1         // Trapped WF* instruction execution.
#define EC_ILLEGAL_EXECUTION_STATE  0xE         // Illegal Execution state.
#define EC_SVC64                    0x15        // SVC instruction execution in AArch64 state.

#define ISS_MASK                    0xFFFFFF

#endif /* EXCEPTION_H */