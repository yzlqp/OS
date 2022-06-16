/**
 * @file log.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef LOG_H
#define LOG_H

#include "fs.h"
#include "../buffer/buf.h"

/**
 * @brief  Reads information about the logging system from the superblock, 
 * including the starting position of the logging area, the size of the logging area.
 * @param  dev: Device number 
 * @param  *sb: superblock pointer to access information
 * @retval None
 */
void initlog(int dev, struct superblock *sb);

/**
 * @brief  Record the block number and pin in the cache by increasing refcnt.
 * @param  *b: The buffer cache to write to 
 * @retval None
 */
void log_write(struct buf *b);

/**
 * @brief  Called at the start of each FS system call.
 * @retval None
 */
void begin_op(void);

/**
 * @brief  called at the end of each FS system call.
 * commits if this was the last outstanding operation.
 * @retval None
 */
void end_op(void);

#endif /* LOG_H */