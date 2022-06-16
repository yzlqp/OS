/**
 * @file kpg.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-01-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../../mmu.h"
#include "memlayout.h"

/*
 * In Linux, the level 4 page table is named PGD -> PUD -> PMD -> PTE
 * Let's stick with the name, and in a 4-level page table, the granularity of each entry is 512G 1G 2M 4K in order   
 * Because at Raspberry PI, the range of physical memory available to us is 0-0x3F000000 total 1008M 
 * And on top of that, you need to define memory type Device
 * So, in order to implement a proper identity mapping, 
 * We need at least level 3 page table granularity to properly configure the memory type 
 */

/*
 * At this granularity, each project represents 2M of memory,
 * The first 204 are common memory and the last 8 are device memory
 */
__attribute__((__aligned__(PGSIZE))) 
uint64_t kpmd[512] = {
    0x0 | PTE_NORMAL_MEMORY,
    0x200000 | PTE_NORMAL_MEMORY,
    0x400000 | PTE_NORMAL_MEMORY,
    0x600000 | PTE_NORMAL_MEMORY,
    0x800000 | PTE_NORMAL_MEMORY,
    0xa00000 | PTE_NORMAL_MEMORY,
    0xc00000 | PTE_NORMAL_MEMORY,
    0xe00000 | PTE_NORMAL_MEMORY,
    0x1000000 | PTE_NORMAL_MEMORY,
    0x1200000 | PTE_NORMAL_MEMORY,
    0x1400000 | PTE_NORMAL_MEMORY,
    0x1600000 | PTE_NORMAL_MEMORY,
    0x1800000 | PTE_NORMAL_MEMORY,
    0x1a00000 | PTE_NORMAL_MEMORY,
    0x1c00000 | PTE_NORMAL_MEMORY,
    0x1e00000 | PTE_NORMAL_MEMORY,
    0x2000000 | PTE_NORMAL_MEMORY,
    0x2200000 | PTE_NORMAL_MEMORY,
    0x2400000 | PTE_NORMAL_MEMORY,
    0x2600000 | PTE_NORMAL_MEMORY,
    0x2800000 | PTE_NORMAL_MEMORY,
    0x2a00000 | PTE_NORMAL_MEMORY,
    0x2c00000 | PTE_NORMAL_MEMORY,
    0x2e00000 | PTE_NORMAL_MEMORY,
    0x3000000 | PTE_NORMAL_MEMORY,
    0x3200000 | PTE_NORMAL_MEMORY,
    0x3400000 | PTE_NORMAL_MEMORY,
    0x3600000 | PTE_NORMAL_MEMORY,
    0x3800000 | PTE_NORMAL_MEMORY,
    0x3a00000 | PTE_NORMAL_MEMORY,
    0x3c00000 | PTE_NORMAL_MEMORY,
    0x3e00000 | PTE_NORMAL_MEMORY,
    0x4000000 | PTE_NORMAL_MEMORY,
    0x4200000 | PTE_NORMAL_MEMORY,
    0x4400000 | PTE_NORMAL_MEMORY,
    0x4600000 | PTE_NORMAL_MEMORY,
    0x4800000 | PTE_NORMAL_MEMORY,
    0x4a00000 | PTE_NORMAL_MEMORY,
    0x4c00000 | PTE_NORMAL_MEMORY,
    0x4e00000 | PTE_NORMAL_MEMORY,
    0x5000000 | PTE_NORMAL_MEMORY,
    0x5200000 | PTE_NORMAL_MEMORY,
    0x5400000 | PTE_NORMAL_MEMORY,
    0x5600000 | PTE_NORMAL_MEMORY,
    0x5800000 | PTE_NORMAL_MEMORY,
    0x5a00000 | PTE_NORMAL_MEMORY,
    0x5c00000 | PTE_NORMAL_MEMORY,
    0x5e00000 | PTE_NORMAL_MEMORY,
    0x6000000 | PTE_NORMAL_MEMORY,
    0x6200000 | PTE_NORMAL_MEMORY,
    0x6400000 | PTE_NORMAL_MEMORY,
    0x6600000 | PTE_NORMAL_MEMORY,
    0x6800000 | PTE_NORMAL_MEMORY,
    0x6a00000 | PTE_NORMAL_MEMORY,
    0x6c00000 | PTE_NORMAL_MEMORY,
    0x6e00000 | PTE_NORMAL_MEMORY,
    0x7000000 | PTE_NORMAL_MEMORY,
    0x7200000 | PTE_NORMAL_MEMORY,
    0x7400000 | PTE_NORMAL_MEMORY,
    0x7600000 | PTE_NORMAL_MEMORY,
    0x7800000 | PTE_NORMAL_MEMORY,
    0x7a00000 | PTE_NORMAL_MEMORY,
    0x7c00000 | PTE_NORMAL_MEMORY,
    0x7e00000 | PTE_NORMAL_MEMORY,
    0x8000000 | PTE_NORMAL_MEMORY,
    0x8200000 | PTE_NORMAL_MEMORY,
    0x8400000 | PTE_NORMAL_MEMORY,
    0x8600000 | PTE_NORMAL_MEMORY,
    0x8800000 | PTE_NORMAL_MEMORY,
    0x8a00000 | PTE_NORMAL_MEMORY,
    0x8c00000 | PTE_NORMAL_MEMORY,
    0x8e00000 | PTE_NORMAL_MEMORY,
    0x9000000 | PTE_NORMAL_MEMORY,
    0x9200000 | PTE_NORMAL_MEMORY,
    0x9400000 | PTE_NORMAL_MEMORY,
    0x9600000 | PTE_NORMAL_MEMORY,
    0x9800000 | PTE_NORMAL_MEMORY,
    0x9a00000 | PTE_NORMAL_MEMORY,
    0x9c00000 | PTE_NORMAL_MEMORY,
    0x9e00000 | PTE_NORMAL_MEMORY,
    0xa000000 | PTE_NORMAL_MEMORY,
    0xa200000 | PTE_NORMAL_MEMORY,
    0xa400000 | PTE_NORMAL_MEMORY,
    0xa600000 | PTE_NORMAL_MEMORY,
    0xa800000 | PTE_NORMAL_MEMORY,
    0xaa00000 | PTE_NORMAL_MEMORY,
    0xac00000 | PTE_NORMAL_MEMORY,
    0xae00000 | PTE_NORMAL_MEMORY,
    0xb000000 | PTE_NORMAL_MEMORY,
    0xb200000 | PTE_NORMAL_MEMORY,
    0xb400000 | PTE_NORMAL_MEMORY,
    0xb600000 | PTE_NORMAL_MEMORY,
    0xb800000 | PTE_NORMAL_MEMORY,
    0xba00000 | PTE_NORMAL_MEMORY,
    0xbc00000 | PTE_NORMAL_MEMORY,
    0xbe00000 | PTE_NORMAL_MEMORY,
    0xc000000 | PTE_NORMAL_MEMORY,
    0xc200000 | PTE_NORMAL_MEMORY,
    0xc400000 | PTE_NORMAL_MEMORY,
    0xc600000 | PTE_NORMAL_MEMORY,
    0xc800000 | PTE_NORMAL_MEMORY,
    0xca00000 | PTE_NORMAL_MEMORY,
    0xcc00000 | PTE_NORMAL_MEMORY,
    0xce00000 | PTE_NORMAL_MEMORY,
    0xd000000 | PTE_NORMAL_MEMORY,
    0xd200000 | PTE_NORMAL_MEMORY,
    0xd400000 | PTE_NORMAL_MEMORY,
    0xd600000 | PTE_NORMAL_MEMORY,
    0xd800000 | PTE_NORMAL_MEMORY,
    0xda00000 | PTE_NORMAL_MEMORY,
    0xdc00000 | PTE_NORMAL_MEMORY,
    0xde00000 | PTE_NORMAL_MEMORY,
    0xe000000 | PTE_NORMAL_MEMORY,
    0xe200000 | PTE_NORMAL_MEMORY,
    0xe400000 | PTE_NORMAL_MEMORY,
    0xe600000 | PTE_NORMAL_MEMORY,
    0xe800000 | PTE_NORMAL_MEMORY,
    0xea00000 | PTE_NORMAL_MEMORY,
    0xec00000 | PTE_NORMAL_MEMORY,
    0xee00000 | PTE_NORMAL_MEMORY,
    0xf000000 | PTE_NORMAL_MEMORY,
    0xf200000 | PTE_NORMAL_MEMORY,
    0xf400000 | PTE_NORMAL_MEMORY,
    0xf600000 | PTE_NORMAL_MEMORY,
    0xf800000 | PTE_NORMAL_MEMORY,
    0xfa00000 | PTE_NORMAL_MEMORY,
    0xfc00000 | PTE_NORMAL_MEMORY,
    0xfe00000 | PTE_NORMAL_MEMORY,
    0x10000000 | PTE_NORMAL_MEMORY,
    0x10200000 | PTE_NORMAL_MEMORY,
    0x10400000 | PTE_NORMAL_MEMORY,
    0x10600000 | PTE_NORMAL_MEMORY,
    0x10800000 | PTE_NORMAL_MEMORY,
    0x10a00000 | PTE_NORMAL_MEMORY,
    0x10c00000 | PTE_NORMAL_MEMORY,
    0x10e00000 | PTE_NORMAL_MEMORY,
    0x11000000 | PTE_NORMAL_MEMORY,
    0x11200000 | PTE_NORMAL_MEMORY,
    0x11400000 | PTE_NORMAL_MEMORY,
    0x11600000 | PTE_NORMAL_MEMORY,
    0x11800000 | PTE_NORMAL_MEMORY,
    0x11a00000 | PTE_NORMAL_MEMORY,
    0x11c00000 | PTE_NORMAL_MEMORY,
    0x11e00000 | PTE_NORMAL_MEMORY,
    0x12000000 | PTE_NORMAL_MEMORY,
    0x12200000 | PTE_NORMAL_MEMORY,
    0x12400000 | PTE_NORMAL_MEMORY,
    0x12600000 | PTE_NORMAL_MEMORY,
    0x12800000 | PTE_NORMAL_MEMORY,
    0x12a00000 | PTE_NORMAL_MEMORY,
    0x12c00000 | PTE_NORMAL_MEMORY,
    0x12e00000 | PTE_NORMAL_MEMORY,
    0x13000000 | PTE_NORMAL_MEMORY,
    0x13200000 | PTE_NORMAL_MEMORY,
    0x13400000 | PTE_NORMAL_MEMORY,
    0x13600000 | PTE_NORMAL_MEMORY,
    0x13800000 | PTE_NORMAL_MEMORY,
    0x13a00000 | PTE_NORMAL_MEMORY,
    0x13c00000 | PTE_NORMAL_MEMORY,
    0x13e00000 | PTE_NORMAL_MEMORY,
    0x14000000 | PTE_NORMAL_MEMORY,
    0x14200000 | PTE_NORMAL_MEMORY,
    0x14400000 | PTE_NORMAL_MEMORY,
    0x14600000 | PTE_NORMAL_MEMORY,
    0x14800000 | PTE_NORMAL_MEMORY,
    0x14a00000 | PTE_NORMAL_MEMORY,
    0x14c00000 | PTE_NORMAL_MEMORY,
    0x14e00000 | PTE_NORMAL_MEMORY,
    0x15000000 | PTE_NORMAL_MEMORY,
    0x15200000 | PTE_NORMAL_MEMORY,
    0x15400000 | PTE_NORMAL_MEMORY,
    0x15600000 | PTE_NORMAL_MEMORY,
    0x15800000 | PTE_NORMAL_MEMORY,
    0x15a00000 | PTE_NORMAL_MEMORY,
    0x15c00000 | PTE_NORMAL_MEMORY,
    0x15e00000 | PTE_NORMAL_MEMORY,
    0x16000000 | PTE_NORMAL_MEMORY,
    0x16200000 | PTE_NORMAL_MEMORY,
    0x16400000 | PTE_NORMAL_MEMORY,
    0x16600000 | PTE_NORMAL_MEMORY,
    0x16800000 | PTE_NORMAL_MEMORY,
    0x16a00000 | PTE_NORMAL_MEMORY,
    0x16c00000 | PTE_NORMAL_MEMORY,
    0x16e00000 | PTE_NORMAL_MEMORY,
    0x17000000 | PTE_NORMAL_MEMORY,
    0x17200000 | PTE_NORMAL_MEMORY,
    0x17400000 | PTE_NORMAL_MEMORY,
    0x17600000 | PTE_NORMAL_MEMORY,
    0x17800000 | PTE_NORMAL_MEMORY,
    0x17a00000 | PTE_NORMAL_MEMORY,
    0x17c00000 | PTE_NORMAL_MEMORY,
    0x17e00000 | PTE_NORMAL_MEMORY,
    0x18000000 | PTE_NORMAL_MEMORY,
    0x18200000 | PTE_NORMAL_MEMORY,
    0x18400000 | PTE_NORMAL_MEMORY,
    0x18600000 | PTE_NORMAL_MEMORY,
    0x18800000 | PTE_NORMAL_MEMORY,
    0x18a00000 | PTE_NORMAL_MEMORY,
    0x18c00000 | PTE_NORMAL_MEMORY,
    0x18e00000 | PTE_NORMAL_MEMORY,
    0x19000000 | PTE_NORMAL_MEMORY,
    0x19200000 | PTE_NORMAL_MEMORY,
    0x19400000 | PTE_NORMAL_MEMORY,
    0x19600000 | PTE_NORMAL_MEMORY,
    0x19800000 | PTE_NORMAL_MEMORY,
    0x19a00000 | PTE_NORMAL_MEMORY,
    0x19c00000 | PTE_NORMAL_MEMORY,
    0x19e00000 | PTE_NORMAL_MEMORY,
    0x1a000000 | PTE_NORMAL_MEMORY,
    0x1a200000 | PTE_NORMAL_MEMORY,
    0x1a400000 | PTE_NORMAL_MEMORY,
    0x1a600000 | PTE_NORMAL_MEMORY,
    0x1a800000 | PTE_NORMAL_MEMORY,
    0x1aa00000 | PTE_NORMAL_MEMORY,
    0x1ac00000 | PTE_NORMAL_MEMORY,
    0x1ae00000 | PTE_NORMAL_MEMORY,
    0x1b000000 | PTE_NORMAL_MEMORY,
    0x1b200000 | PTE_NORMAL_MEMORY,
    0x1b400000 | PTE_NORMAL_MEMORY,
    0x1b600000 | PTE_NORMAL_MEMORY,
    0x1b800000 | PTE_NORMAL_MEMORY,
    0x1ba00000 | PTE_NORMAL_MEMORY,
    0x1bc00000 | PTE_NORMAL_MEMORY,
    0x1be00000 | PTE_NORMAL_MEMORY,
    0x1c000000 | PTE_NORMAL_MEMORY,
    0x1c200000 | PTE_NORMAL_MEMORY,
    0x1c400000 | PTE_NORMAL_MEMORY,
    0x1c600000 | PTE_NORMAL_MEMORY,
    0x1c800000 | PTE_NORMAL_MEMORY,
    0x1ca00000 | PTE_NORMAL_MEMORY,
    0x1cc00000 | PTE_NORMAL_MEMORY,
    0x1ce00000 | PTE_NORMAL_MEMORY,
    0x1d000000 | PTE_NORMAL_MEMORY,
    0x1d200000 | PTE_NORMAL_MEMORY,
    0x1d400000 | PTE_NORMAL_MEMORY,
    0x1d600000 | PTE_NORMAL_MEMORY,
    0x1d800000 | PTE_NORMAL_MEMORY,
    0x1da00000 | PTE_NORMAL_MEMORY,
    0x1dc00000 | PTE_NORMAL_MEMORY,
    0x1de00000 | PTE_NORMAL_MEMORY,
    0x1e000000 | PTE_NORMAL_MEMORY,
    0x1e200000 | PTE_NORMAL_MEMORY,
    0x1e400000 | PTE_NORMAL_MEMORY,
    0x1e600000 | PTE_NORMAL_MEMORY,
    0x1e800000 | PTE_NORMAL_MEMORY,
    0x1ea00000 | PTE_NORMAL_MEMORY,
    0x1ec00000 | PTE_NORMAL_MEMORY,
    0x1ee00000 | PTE_NORMAL_MEMORY,
    0x1f000000 | PTE_NORMAL_MEMORY,
    0x1f200000 | PTE_NORMAL_MEMORY,
    0x1f400000 | PTE_NORMAL_MEMORY,
    0x1f600000 | PTE_NORMAL_MEMORY,
    0x1f800000 | PTE_NORMAL_MEMORY,
    0x1fa00000 | PTE_NORMAL_MEMORY,
    0x1fc00000 | PTE_NORMAL_MEMORY,
    0x1fe00000 | PTE_NORMAL_MEMORY,
    0x20000000 | PTE_NORMAL_MEMORY,
    0x20200000 | PTE_NORMAL_MEMORY,
    0x20400000 | PTE_NORMAL_MEMORY,
    0x20600000 | PTE_NORMAL_MEMORY,
    0x20800000 | PTE_NORMAL_MEMORY,
    0x20a00000 | PTE_NORMAL_MEMORY,
    0x20c00000 | PTE_NORMAL_MEMORY,
    0x20e00000 | PTE_NORMAL_MEMORY,
    0x21000000 | PTE_NORMAL_MEMORY,
    0x21200000 | PTE_NORMAL_MEMORY,
    0x21400000 | PTE_NORMAL_MEMORY,
    0x21600000 | PTE_NORMAL_MEMORY,
    0x21800000 | PTE_NORMAL_MEMORY,
    0x21a00000 | PTE_NORMAL_MEMORY,
    0x21c00000 | PTE_NORMAL_MEMORY,
    0x21e00000 | PTE_NORMAL_MEMORY,
    0x22000000 | PTE_NORMAL_MEMORY,
    0x22200000 | PTE_NORMAL_MEMORY,
    0x22400000 | PTE_NORMAL_MEMORY,
    0x22600000 | PTE_NORMAL_MEMORY,
    0x22800000 | PTE_NORMAL_MEMORY,
    0x22a00000 | PTE_NORMAL_MEMORY,
    0x22c00000 | PTE_NORMAL_MEMORY,
    0x22e00000 | PTE_NORMAL_MEMORY,
    0x23000000 | PTE_NORMAL_MEMORY,
    0x23200000 | PTE_NORMAL_MEMORY,
    0x23400000 | PTE_NORMAL_MEMORY,
    0x23600000 | PTE_NORMAL_MEMORY,
    0x23800000 | PTE_NORMAL_MEMORY,
    0x23a00000 | PTE_NORMAL_MEMORY,
    0x23c00000 | PTE_NORMAL_MEMORY,
    0x23e00000 | PTE_NORMAL_MEMORY,
    0x24000000 | PTE_NORMAL_MEMORY,
    0x24200000 | PTE_NORMAL_MEMORY,
    0x24400000 | PTE_NORMAL_MEMORY,
    0x24600000 | PTE_NORMAL_MEMORY,
    0x24800000 | PTE_NORMAL_MEMORY,
    0x24a00000 | PTE_NORMAL_MEMORY,
    0x24c00000 | PTE_NORMAL_MEMORY,
    0x24e00000 | PTE_NORMAL_MEMORY,
    0x25000000 | PTE_NORMAL_MEMORY,
    0x25200000 | PTE_NORMAL_MEMORY,
    0x25400000 | PTE_NORMAL_MEMORY,
    0x25600000 | PTE_NORMAL_MEMORY,
    0x25800000 | PTE_NORMAL_MEMORY,
    0x25a00000 | PTE_NORMAL_MEMORY,
    0x25c00000 | PTE_NORMAL_MEMORY,
    0x25e00000 | PTE_NORMAL_MEMORY,
    0x26000000 | PTE_NORMAL_MEMORY,
    0x26200000 | PTE_NORMAL_MEMORY,
    0x26400000 | PTE_NORMAL_MEMORY,
    0x26600000 | PTE_NORMAL_MEMORY,
    0x26800000 | PTE_NORMAL_MEMORY,
    0x26a00000 | PTE_NORMAL_MEMORY,
    0x26c00000 | PTE_NORMAL_MEMORY,
    0x26e00000 | PTE_NORMAL_MEMORY,
    0x27000000 | PTE_NORMAL_MEMORY,
    0x27200000 | PTE_NORMAL_MEMORY,
    0x27400000 | PTE_NORMAL_MEMORY,
    0x27600000 | PTE_NORMAL_MEMORY,
    0x27800000 | PTE_NORMAL_MEMORY,
    0x27a00000 | PTE_NORMAL_MEMORY,
    0x27c00000 | PTE_NORMAL_MEMORY,
    0x27e00000 | PTE_NORMAL_MEMORY,
    0x28000000 | PTE_NORMAL_MEMORY,
    0x28200000 | PTE_NORMAL_MEMORY,
    0x28400000 | PTE_NORMAL_MEMORY,
    0x28600000 | PTE_NORMAL_MEMORY,
    0x28800000 | PTE_NORMAL_MEMORY,
    0x28a00000 | PTE_NORMAL_MEMORY,
    0x28c00000 | PTE_NORMAL_MEMORY,
    0x28e00000 | PTE_NORMAL_MEMORY,
    0x29000000 | PTE_NORMAL_MEMORY,
    0x29200000 | PTE_NORMAL_MEMORY,
    0x29400000 | PTE_NORMAL_MEMORY,
    0x29600000 | PTE_NORMAL_MEMORY,
    0x29800000 | PTE_NORMAL_MEMORY,
    0x29a00000 | PTE_NORMAL_MEMORY,
    0x29c00000 | PTE_NORMAL_MEMORY,
    0x29e00000 | PTE_NORMAL_MEMORY,
    0x2a000000 | PTE_NORMAL_MEMORY,
    0x2a200000 | PTE_NORMAL_MEMORY,
    0x2a400000 | PTE_NORMAL_MEMORY,
    0x2a600000 | PTE_NORMAL_MEMORY,
    0x2a800000 | PTE_NORMAL_MEMORY,
    0x2aa00000 | PTE_NORMAL_MEMORY,
    0x2ac00000 | PTE_NORMAL_MEMORY,
    0x2ae00000 | PTE_NORMAL_MEMORY,
    0x2b000000 | PTE_NORMAL_MEMORY,
    0x2b200000 | PTE_NORMAL_MEMORY,
    0x2b400000 | PTE_NORMAL_MEMORY,
    0x2b600000 | PTE_NORMAL_MEMORY,
    0x2b800000 | PTE_NORMAL_MEMORY,
    0x2ba00000 | PTE_NORMAL_MEMORY,
    0x2bc00000 | PTE_NORMAL_MEMORY,
    0x2be00000 | PTE_NORMAL_MEMORY,
    0x2c000000 | PTE_NORMAL_MEMORY,
    0x2c200000 | PTE_NORMAL_MEMORY,
    0x2c400000 | PTE_NORMAL_MEMORY,
    0x2c600000 | PTE_NORMAL_MEMORY,
    0x2c800000 | PTE_NORMAL_MEMORY,
    0x2ca00000 | PTE_NORMAL_MEMORY,
    0x2cc00000 | PTE_NORMAL_MEMORY,
    0x2ce00000 | PTE_NORMAL_MEMORY,
    0x2d000000 | PTE_NORMAL_MEMORY,
    0x2d200000 | PTE_NORMAL_MEMORY,
    0x2d400000 | PTE_NORMAL_MEMORY,
    0x2d600000 | PTE_NORMAL_MEMORY,
    0x2d800000 | PTE_NORMAL_MEMORY,
    0x2da00000 | PTE_NORMAL_MEMORY,
    0x2dc00000 | PTE_NORMAL_MEMORY,
    0x2de00000 | PTE_NORMAL_MEMORY,
    0x2e000000 | PTE_NORMAL_MEMORY,
    0x2e200000 | PTE_NORMAL_MEMORY,
    0x2e400000 | PTE_NORMAL_MEMORY,
    0x2e600000 | PTE_NORMAL_MEMORY,
    0x2e800000 | PTE_NORMAL_MEMORY,
    0x2ea00000 | PTE_NORMAL_MEMORY,
    0x2ec00000 | PTE_NORMAL_MEMORY,
    0x2ee00000 | PTE_NORMAL_MEMORY,
    0x2f000000 | PTE_NORMAL_MEMORY,
    0x2f200000 | PTE_NORMAL_MEMORY,
    0x2f400000 | PTE_NORMAL_MEMORY,
    0x2f600000 | PTE_NORMAL_MEMORY,
    0x2f800000 | PTE_NORMAL_MEMORY,
    0x2fa00000 | PTE_NORMAL_MEMORY,
    0x2fc00000 | PTE_NORMAL_MEMORY,
    0x2fe00000 | PTE_NORMAL_MEMORY,
    0x30000000 | PTE_NORMAL_MEMORY,
    0x30200000 | PTE_NORMAL_MEMORY,
    0x30400000 | PTE_NORMAL_MEMORY,
    0x30600000 | PTE_NORMAL_MEMORY,
    0x30800000 | PTE_NORMAL_MEMORY,
    0x30a00000 | PTE_NORMAL_MEMORY,
    0x30c00000 | PTE_NORMAL_MEMORY,
    0x30e00000 | PTE_NORMAL_MEMORY,
    0x31000000 | PTE_NORMAL_MEMORY,
    0x31200000 | PTE_NORMAL_MEMORY,
    0x31400000 | PTE_NORMAL_MEMORY,
    0x31600000 | PTE_NORMAL_MEMORY,
    0x31800000 | PTE_NORMAL_MEMORY,
    0x31a00000 | PTE_NORMAL_MEMORY,
    0x31c00000 | PTE_NORMAL_MEMORY,
    0x31e00000 | PTE_NORMAL_MEMORY,
    0x32000000 | PTE_NORMAL_MEMORY,
    0x32200000 | PTE_NORMAL_MEMORY,
    0x32400000 | PTE_NORMAL_MEMORY,
    0x32600000 | PTE_NORMAL_MEMORY,
    0x32800000 | PTE_NORMAL_MEMORY,
    0x32a00000 | PTE_NORMAL_MEMORY,
    0x32c00000 | PTE_NORMAL_MEMORY,
    0x32e00000 | PTE_NORMAL_MEMORY,
    0x33000000 | PTE_NORMAL_MEMORY,
    0x33200000 | PTE_NORMAL_MEMORY,
    0x33400000 | PTE_NORMAL_MEMORY,
    0x33600000 | PTE_NORMAL_MEMORY,
    0x33800000 | PTE_NORMAL_MEMORY,
    0x33a00000 | PTE_NORMAL_MEMORY,
    0x33c00000 | PTE_NORMAL_MEMORY,
    0x33e00000 | PTE_NORMAL_MEMORY,
    0x34000000 | PTE_NORMAL_MEMORY,
    0x34200000 | PTE_NORMAL_MEMORY,
    0x34400000 | PTE_NORMAL_MEMORY,
    0x34600000 | PTE_NORMAL_MEMORY,
    0x34800000 | PTE_NORMAL_MEMORY,
    0x34a00000 | PTE_NORMAL_MEMORY,
    0x34c00000 | PTE_NORMAL_MEMORY,
    0x34e00000 | PTE_NORMAL_MEMORY,
    0x35000000 | PTE_NORMAL_MEMORY,
    0x35200000 | PTE_NORMAL_MEMORY,
    0x35400000 | PTE_NORMAL_MEMORY,
    0x35600000 | PTE_NORMAL_MEMORY,
    0x35800000 | PTE_NORMAL_MEMORY,
    0x35a00000 | PTE_NORMAL_MEMORY,
    0x35c00000 | PTE_NORMAL_MEMORY,
    0x35e00000 | PTE_NORMAL_MEMORY,
    0x36000000 | PTE_NORMAL_MEMORY,
    0x36200000 | PTE_NORMAL_MEMORY,
    0x36400000 | PTE_NORMAL_MEMORY,
    0x36600000 | PTE_NORMAL_MEMORY,
    0x36800000 | PTE_NORMAL_MEMORY,
    0x36a00000 | PTE_NORMAL_MEMORY,
    0x36c00000 | PTE_NORMAL_MEMORY,
    0x36e00000 | PTE_NORMAL_MEMORY,
    0x37000000 | PTE_NORMAL_MEMORY,
    0x37200000 | PTE_NORMAL_MEMORY,
    0x37400000 | PTE_NORMAL_MEMORY,
    0x37600000 | PTE_NORMAL_MEMORY,
    0x37800000 | PTE_NORMAL_MEMORY,
    0x37a00000 | PTE_NORMAL_MEMORY,
    0x37c00000 | PTE_NORMAL_MEMORY,
    0x37e00000 | PTE_NORMAL_MEMORY,
    0x38000000 | PTE_NORMAL_MEMORY,
    0x38200000 | PTE_NORMAL_MEMORY,
    0x38400000 | PTE_NORMAL_MEMORY,
    0x38600000 | PTE_NORMAL_MEMORY,
    0x38800000 | PTE_NORMAL_MEMORY,
    0x38a00000 | PTE_NORMAL_MEMORY,
    0x38c00000 | PTE_NORMAL_MEMORY,
    0x38e00000 | PTE_NORMAL_MEMORY,
    0x39000000 | PTE_NORMAL_MEMORY,
    0x39200000 | PTE_NORMAL_MEMORY,
    0x39400000 | PTE_NORMAL_MEMORY,
    0x39600000 | PTE_NORMAL_MEMORY,
    0x39800000 | PTE_NORMAL_MEMORY,
    0x39a00000 | PTE_NORMAL_MEMORY,
    0x39c00000 | PTE_NORMAL_MEMORY,
    0x39e00000 | PTE_NORMAL_MEMORY,
    0x3a000000 | PTE_NORMAL_MEMORY,
    0x3a200000 | PTE_NORMAL_MEMORY,
    0x3a400000 | PTE_NORMAL_MEMORY,
    0x3a600000 | PTE_NORMAL_MEMORY,
    0x3a800000 | PTE_NORMAL_MEMORY,
    0x3aa00000 | PTE_NORMAL_MEMORY,
    0x3ac00000 | PTE_NORMAL_MEMORY,
    0x3ae00000 | PTE_NORMAL_MEMORY,
    0x3b000000 | PTE_NORMAL_MEMORY,
    0x3b200000 | PTE_NORMAL_MEMORY,
    0x3b400000 | PTE_NORMAL_MEMORY,
    0x3b600000 | PTE_NORMAL_MEMORY,
    0x3b800000 | PTE_NORMAL_MEMORY,
    0x3ba00000 | PTE_NORMAL_MEMORY,
    0x3bc00000 | PTE_NORMAL_MEMORY,
    0x3be00000 | PTE_NORMAL_MEMORY,
    0x3c000000 | PTE_NORMAL_MEMORY,
    0x3c200000 | PTE_NORMAL_MEMORY,
    0x3c400000 | PTE_NORMAL_MEMORY,
    0x3c600000 | PTE_NORMAL_MEMORY,
    0x3c800000 | PTE_NORMAL_MEMORY,
    0x3ca00000 | PTE_NORMAL_MEMORY,
    0x3cc00000 | PTE_NORMAL_MEMORY,
    0x3ce00000 | PTE_NORMAL_MEMORY,
    0x3d000000 | PTE_NORMAL_MEMORY,
    0x3d200000 | PTE_NORMAL_MEMORY,
    0x3d400000 | PTE_NORMAL_MEMORY,
    0x3d600000 | PTE_NORMAL_MEMORY,
    0x3d800000 | PTE_NORMAL_MEMORY,
    0x3da00000 | PTE_NORMAL_MEMORY,
    0x3dc00000 | PTE_NORMAL_MEMORY,
    0x3de00000 | PTE_NORMAL_MEMORY,
    0x3e000000 | PTE_NORMAL_MEMORY,
    0x3e200000 | PTE_NORMAL_MEMORY,
    0x3e400000 | PTE_NORMAL_MEMORY,
    0x3e600000 | PTE_NORMAL_MEMORY,
    0x3e800000 | PTE_NORMAL_MEMORY,
    0x3ea00000 | PTE_NORMAL_MEMORY,
    0x3ec00000 | PTE_NORMAL_MEMORY,
    0x3ee00000 | PTE_NORMAL_MEMORY,
    0x3f000000 | PTE_DEVICE_MEMORY,
    0x3f200000 | PTE_DEVICE_MEMORY,
    0x3f400000 | PTE_DEVICE_MEMORY,
    0x3f600000 | PTE_DEVICE_MEMORY,
    0x3f800000 | PTE_DEVICE_MEMORY,
    0x3fa00000 | PTE_DEVICE_MEMORY,
    0x3fc00000 | PTE_DEVICE_MEMORY,
    0x3fe00000 | PTE_DEVICE_MEMORY
};

/*
 * At this granularity, each project represents 1 gb of memory.
 * In 0-1G memory points to KPMD can be, 1G-2G is set to Device memory.
 */
__attribute__((__aligned__(PGSIZE)))
uint64_t kpud[512] = {
    VA2PA(kpmd) + PTE_TABLE,
    0x40000000 + PTE_DEVICE_MEMORY
};

/*
 * At this granularity, 512 GB of memory per project
 */
__attribute__((__aligned__(PGSIZE)))
uint64_t kpgd[512] = {
    VA2PA(kpud) + PTE_TABLE
};