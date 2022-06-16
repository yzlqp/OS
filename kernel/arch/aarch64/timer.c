/**
 * @file timer.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "timer.h"
#include "proc/proc.h"
#include "printf.h"
#include "arm.h"
#include "board/raspi3/irq.h"
#include "board/raspi3/local_peripherals.h"
#include "../../sync/spinlock.h"

#define CNTP_CTL_EL0_ENABLE     1
#define CNTP_CTL_EL0_IMASK      (1 << 1)
#define CNTP_CTL_EL0_ISTATUS    (1 << 2)

static uint64_t _timer_unit;
struct spinlock tickslock;
uint64_t ticks;

/*
 * Timer interrupt handler
 */
void clock_intr()
{
    acquire_spin_lock(&tickslock);
    ticks++;
    wakeup(&ticks);
    release_spin_lock(&tickslock);
}

/*
 * Gets the value of the current ticks 
 */
uint64_t uptime()
{
    uint64_t xticks;
    acquire_spin_lock(&tickslock);
    xticks = ticks;
    release_spin_lock(&tickslock);
    return xticks;
}

/*
 * Initialize the Arm Generic Timer
 */
void timer_init()
{
    init_spin_lock(&tickslock, "tickslock");
    ticks = 0;

    uint64_t timer_frq = r_cntfrq_el0();
    cprintf("[cpu %d] timer frequency: %lu\n", cpuid(), timer_frq);
    // The minimum time unit is 1ms
    _timer_unit = timer_frq / 1000;    // TODO float
    timer_tick_in(1000);
    // The corresponding Core timer is enabled
    put32(COREn_TIMER_INTERRUPT_CONTROL(cpuid()), CORE_TIMER_ENABLE); 
    l_cntp_ctl_el0(CNTP_CTL_EL0_ENABLE);
}

/*
 * Set the next trigger time
 */
void timer_tick_in(uint64_t ms)
{
    uint64_t value = ms * _timer_unit;
    l_cntp_tval_el0(value);
}

/*
 * Reset timer, Temporarily set a time slice of 0.1s
 */
void timer_reset()
{
    timer_tick_in(100); 
}
