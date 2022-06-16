/**
 * @file sysreg.h
 * @author ylp
 * @brief Values for each CONTROL register of the ARM CPU will be configured
 * @version 0.1
 * @date 2021-12-25
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARM_SYSREG_H
#define ARM_SYSREG_H

/*
 * SCR_EL3 Secure Configuration Register EL3
 * https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/SCR-EL3--Secure-Configuration-Register
 * [0] 1 NS none secure EL0 and EL1 are in non-secure state
 * [1] 0 IRQ routing When set to 0 (EL3 is not substituted for IRQ generated below EL3 level; When in EL3, IRQ is not generated.), When set to 1: Whatever EL is generated, the IQR will be substituted into EL3
 * [2] 0 FIQ routing The same as the above IRQ route
 * [3] 0 EA routing  Controls Abort and SErro routing
 * [5:4] 1 RES1 Set the reserved bit to 1
 * [6] 0 RES0 Set the reserved bit to 0
 * [7] 1 SMD Secure Monitor Call Disable, SMC instruction at EL1 level and above(Secure Monitor Call)
 * [8] 1 HCE Hypervisor Call instruction enable. The HVC command is enabled
 * [9] 0 SIF Secure instruction fetch Allows safe-state instructions to be fetched from memory marked as unsafe when translating stage1 addresses，1 is not allowed, we do not use Arm secure, so 0 is ok
 * [10] 1 If this bit is 0, only AARCH32 can be used at levels lower than EL3, and AARCH64 can be used after 1
 */
#define SCR_NS               (1)
#define SCR_EXCEPTION_ROUTE  (0 << 1)
#define SCR_RES1             (0b11 << 4)
#define SCR_RES0             (0 << 6)
#define SCR_SMD_DISABLE      (1 << 7)
#define SCR_HCE_ENABLE       (1 << 8)
#define SCR_SIF              (0 << 9)
#define SCR_RW               (1 << 10)

/*
 * The SCR VALUE should be 0b0101_1011_0001  0x05B1
 */ 
#define SCR_VALUE                                         \
    (SCR_NS | SCR_EXCEPTION_ROUTE | SCR_RES1 | SCR_RES0 | \
     SCR_SMD_DISABLE | SCR_HCE_ENABLE | SCR_SIF | SCR_RW)

/*
 * SPSR_ELx Saved Program Status Register
 * https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/SPSR-EL3--Saved-Program-Status-Register--EL3-
 * This register holds CPSR_EL(x-1), so when using the ERET instruction, 
 * This register state controls the state returned to EL(X-1)  
 * Write data to SPSR EL3 0b0001_1100_1001
 * [3:0]:1001 
 * [4]:0 This bit is 0, indicating that you were working in aARCH64 state before entering this EL
 * [5]:0 RES0
 * [6]:1 FIQ interrupt mask When jumping to EL2, this bit is copied to the PSTATE of EL2 and FIQ interrupts are turned off
 * [7]:1 IRQ mask
 * [8]:1 SError mask
 * [9]:0 Debug mask
 */
#define SPSR_MASK_FIS   (0b111 << 6)
#define SPSR_EL1h       (0b0101 << 0)
#define SPSR_EL2h       (0b1001 << 0)
// SPSR_EL3_VALUE is 0b0001_1100_1001 0x1C9 457
#define SPSR_EL3_VALUE  (SPSR_MASK_FIS | SPSR_EL2h)
// SPSR_EL2_VALUE is 0b0001_1100_0101 0x1C5 453
#define SPSR_EL2_VALUE  (SPSR_MASK_FIS | SPSR_EL1h)

/*
 * SCTLR_EL1, System Control Register (EL1). 
 */ 
#define SCTLR_RESERVED \
    ((3 << 28) | (3 << 22) | (1 << 20) | (1 << 11) | (1 << 8) | (1 << 7))
#define SCTLR_EE_LITTLE_ENDIAN  (0 << 25)
#define SCTLR_E0E_LITTLE_ENDIAN (0 << 24)
#define SCTLR_I_CACHE           (1 << 12)
#define SCTLR_D_CACHE           (1 << 2)
#define SCTLR_MMU_DISABLED      (0 << 0)
#define SCTLR_MMU_ENABLED       (1 << 0)

#define SCTLR_VALUE_MMU_DISABLED_VALUE \
    (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_E0E_LITTLE_ENDIAN | SCTLR_I_CACHE | SCTLR_D_CACHE | SCTLR_MMU_DISABLED)

#endif /* ARM_SYSREG_H */