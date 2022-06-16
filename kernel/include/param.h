/**
 * @file param.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef PARAM_H
#define PARAM_H

#define NCPU        4        // the number of cpu
#define NPROC       64       // Maximum number of processes
#define NOFILE      16       // Maximum number of files that can be opened by each process
#define KSTACKSIZE  4096     // The size of the kernel stack per process

#define MAXOPBLOCKS 10       // The maximum number of blocks allowed per transaction
#define NBUF        (MAXOPBLOCKS * 3) // Buffer Size
#define LOGSIZE     (MAXOPBLOCKS * 3)

#define NINODE      50      // Maximum number of active inodes
#define NFILE       100     // Maximum number of files that can be opened by the operating system
#define NDEV        10      // Maximum number of devices
#define ROOTDEV     1       // device number of file system root disk
#define MAXARG      32      // max exec arguments
#define INPUT_BUF   128

#define MAXPATH     128     // maximum file path name

// mkfs only
#define FSSIZE      1000    // Size of file system in blocks

#endif /* PARAM_H */