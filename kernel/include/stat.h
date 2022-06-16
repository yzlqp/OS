/**
 * @file stat.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-04-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef STAT_H
#define STAT_H

#include "stdint.h"

#define T_DIR     1    // Directory
#define T_FILE    2    // File
#define T_DEVICE  3    // Device

struct stat {
	int dev;           // device number
	uint32_t ino;      // Inode number
	short type;        // File type, files, directories, or special files (devices), 0 indicates dinode is free  
	short nlink;       // (Number of hard links) Number of links to inode in file system, won’t free an inode if its link count is greater than 0 
	uint64_t size;     // The number of bytes of content in the file
};

#endif /* STAT_H */