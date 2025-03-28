// Copyright 2020-2021 Beken
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "bk_log.h"

#define SD_STATIC static

#define SD_CARD_TAG "sd_card"
#define SD_CARD_LOGI(...) BK_LOGI(SD_CARD_TAG, ##__VA_ARGS__)
#define SD_CARD_LOGW(...) BK_LOGW(SD_CARD_TAG, ##__VA_ARGS__)
#define SD_CARD_LOGE(...) BK_LOGE(SD_CARD_TAG, ##__VA_ARGS__)
#define SD_CARD_LOGD(...) BK_LOGD(SD_CARD_TAG, ##__VA_ARGS__)

/* sd card supported version */
#define SD_CARD_V1_X     0x0
#define SD_CARD_V2_X     0x1

/* sd supported memory cards */
#define SD_CARD_TYPE_SDSC       0
#define SD_CARD_TYPE_SDHC_SDXC  1

/* standard sd commands */
#define SD_CMD_GO_IDLE_STATE         0
#define SD_CMD_ALL_SEND_CID          2
#define SD_CMD_SEND_RELATIVE_ADDR    3   /* R6 */
#define SD_CMD_SELECT_DESELECT_CARD  7   /* R1b */
#define SD_CMD_SEND_IF_COND          8   /* R7 */
#define SD_CMD_SEND_CSD              9   /* R2 */
#define SD_CMD_STOP_TRANSMISSION     12  /* R1b */
#define SD_CMD_SEND_STATUS           13
#define SD_CMD_SET_BLOCKLEN          16  /* R1 */
#define SD_CMD_READ_SINGLE_BLOCK     17  /* R1 */
#define SD_CMD_READ_MULTIPLE_BLOCK   18  /* R1 */
#define SD_CMD_WRITE_BLOCK           24  /* R1 */
#define SD_CMD_WRITE_MULTIPLE_BLOCK  25  /* R1 */
#define SD_CMD_ERASE_WR_BLK_START    32
#define SD_CMD_ERASE_WR_BLK_END      33
#define SD_CMD_ERASE                 38
#define SD_CMD_SD_SEND_OP_COND       41  /* R3 */
#define SD_CMD_APP_CMD               55  /* R1 */
#define SD_CMD_APP_CMD6_SET_BUS_WIDTH 6  /* After CMD55 */

#define SD_CHECK_PATTERN          0x1AA

/* OCR register */
#define SD_OCR_BUSY               BIT(31) /* bit[31]: busy */
#define SD_OCR_HIGH_CAPACITY      BIT(30) /* bit[30] */
#define SD_OCR_VOLTAGE_3_2V_3_3V  0x00100000 // Voltage 3.2V to 3.3V flag
#define SD_OCR_VOLTAGE_ALL        0x00FF8000 // All Voltage flag
#define SD_DEFAULT_OCR            (SD_OCR_VOLTAGE_ALL | SD_OCR_HIGH_CAPACITY)

#define SD_BLOCK_SIZE             512

/* Command class support */
#define SD_SUPPORT_BASIC_CMD      	BIT(0)
#define SD_SUPPORT_RESERVED_0_CMD   BIT(1)
#define SD_SUPPORT_BLOCK_READ_CMD   BIT(2)
#define SD_SUPPORT_RESERVED_1_CMD   BIT(3)
#define SD_SUPPORT_BLOCK_WRITE_CMD  BIT(4)
#define SD_SUPPORT_ERASE_CMD      	BIT(5)
#define SD_SUPPORT_WRITE_PROTECTION_CMD   BIT(6)
#define SD_SUPPORT_LOCK_CARD_CMD   	BIT(7)
#define SD_SUPPORT_APPLICATION_CMD  BIT(8)
#define SD_SUPPORT_RESERVED_2_CMD   (BIT(9) | BIT(10) | BIT(11))

#define SD_CARD_MAX_VOLT_TRIAL_COUNT    0xff
#define SD_CARD_MAX_CMD_TRIAL_COUNT	    (128)
#define SD_CARD_MAX_DATA_TRIAL_COUNT	(128)

#if (CONFIG_SDIO_V2P0)	//Temp code, clock module should re-arch.
#if (CFG_SOC_NAME == SOC_BK7252N)
#define DATA_TIMEOUT_203K	 20600 //100ms
#define DATA_TIMEOUT_6_5_M   3000000 //450ms
#define DATA_TIMEOUT_13_M    6000000 //450ms
#define DATA_TIMEOUT_26_M    12000000 //450ms
#define DATA_TIMEOUT_937K    (461600)
#define DATA_TIMEOUT_30_M    (15000000)
#define DATA_TIMEOUT_60_M    (30000000)
#define DATA_TIMEOUT_120_M   (60000000)

#define CMD_TIMEOUT_203K    (5075)
#define CMD_TIMEOUT_6_5_M   (300000)
#define CMD_TIMEOUT_13_M    (600000)
#define CMD_TIMEOUT_26_M    (1200000)
#define CMD_TIMEOUT_937K    (46200)
#define CMD_TIMEOUT_30_M    (1384625)
#define CMD_TIMEOUT_60_M    (2769250)
#define CMD_TIMEOUT_120_M   (5538500)
#else
#define CMD_TIMEOUT_100K	2500
#define DATA_TIMEOUT_100K	10000

#define CMD_TIMEOUT_6_5_M	6500
#define DATA_TIMEOUT_6_5_M  208000

#define CMD_TIMEOUT_13M		13000	//1ms, max retry_cnt 128(SD_CARD_MAX_CMD_TRIAL_COUNT)
#define DATA_TIMEOUT_13M	416000	//32ms, max retry_cnt 128(SD_CARD_MAX_DATA_TRIAL_COUNT)

#define CMD_TIMEOUT_20M		20000	//1ms, max retry_cnt 128
#define DATA_TIMEOUT_20M	640000	//32ms, max retry_cnt 128

#define CMD_TIMEOUT_40M		4000	//1ms, max retry_cnt 128
#define DATA_TIMEOUT_40M	1280000	//32ms, max retry_cnt 128

#define CMD_TIMEOUT_80M		80000	//1ms, read/write file 60000 times, the max wait is about 2.4ms(cmd retry max count==24)
#define DATA_TIMEOUT_80M	2560000	//32ms, max retry_cnt 128:some SDCARD(Sandisk 16G,Class-4) after read 10M data later,read a block takes more then 16ms

#define CMD_TIMEOUT_200K	5000	//about 5us per cycle (25ms)
#define DATA_TIMEOUT_200K	20000 //100ms
#endif // (CFG_SOC_NAME == SOC_BK7252N)
#else
#define CMD_TIMEOUT_200K	5000	//about 5us per cycle (25ms)
#define DATA_TIMEOUT_200K	20000 //100ms

#define CMD_TIMEOUT_6_5_M	300000 //about 150ns per cycle (45ms)
#define DATA_TIMEOUT_6_5_M  3000000 //450ms

#define CMD_TIMEOUT_13M		600000 //about 77ns pr cycle (45ms)
#define DATA_TIMEOUT_13M	6000000 //450ms

#define CMD_TIMEOUT_26M		1200000//about 38ns pr cycle (45ms)
#define DATA_TIMEOUT_26M	12000000 //450ms
#endif
// eof

