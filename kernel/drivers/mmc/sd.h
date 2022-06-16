/**
 * @file sd.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef SD_H
#define SD_H

#include "buffer/buf.h"

#define SD_OK              0
#define SD_ERROR           1
#define SD_TIMEOUT         2
#define SD_BUSY            3
#define SD_NO_RESP         5
#define SD_ERROR_RESET     6
#define SD_ERROR_CLOCK     7
#define SD_ERROR_VOLTAGE   8
#define SD_ERROR_APP_CMD   9
#define SD_CARD_CHANGED    10
#define SD_CARD_ABSENT     11
#define SD_CARD_REINSERTED 12

#define SD_READ_BLOCKS     0
#define SD_WRITE_BLOCKS    1

/**
 * @brief Initialize SD card and parse MBR.
 * @retval None
 */
void sd_init();

/**
 * @brief The SD interrupt handler.
 * @retval None
 */
void sd_intr();

/**
 * @brief Sync buf with disk.
 * If BUF_DIRTY is set, write buf to disk, clear BUF_DIRTY, set BUF_VALID.
 * Else if BUF_VALID is not set, read buf from disk, set BUF_VALID.
 * @retval None
 */
void sd_rw(struct buf *);

/**
 * @brief SD card test and benchmark
 * @retval None
 */
void sd_test();

#endif /* SD_H */