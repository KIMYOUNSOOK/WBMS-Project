/*******************************************************************************
 * @file adi_wil_example_cell_balance.h
 *
 * Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved. This
 * software is proprietary and confidential to Analog Devices, Inc. and its
 * licensors.
 *******************************************************************************/

#ifndef _ADI_WIL_EXAMPLE_CELL_BALANCE_H
#define _ADI_WIL_EXAMPLE_CELL_BALANCE_H

#include <stdint.h>
#include <stdbool.h>

#include "adi_wil_types.h"
#include "adi_wil_script_change.h"
#include "adi_wil_api.h"
#include "adi_wil_example_debug_functions.h"

/*******************************************************************************/
/* #defines                                                                    */
/*******************************************************************************/
#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
#define BMS_SCRIPT_WRCFGB_OFFSET                    441u  /* changes with script */
#define BMS_PAYLOAD_EXCLUDE_WRCFGB_LENGTH           54    /* changes with script */
#define BMS_PAYLOAD_WRCFGB_PACKET_ID                3     /* changes with script */
#define BMS_PACKETS_PER_INTERVAL                    3     /* changes with script */
#define WRCFGB_PEC_OFFSET                           6     /* This offset is only for a single BMS IC */
#define WRCFGB_DCC_OFFSET                           4     /* This offset is only for a single BMS IC */
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
#define BMS_SCRIPT_WRCFGB_OFFSET                 421  // 463u  /* changes with script */
#define BMS_SCRIPT_WRCFGA_OFFSET                 406 //  448u  /* changes with script */
#define BMS_PAYLOAD_EXCLUDE_WRCFGB_LENGTH           6     /* changes with script */
#define BMS_PAYLOAD_WRCFGB_PACKET_ID                3     /* changes with script */
#define BMS_PACKETS_PER_INTERVAL                    3     /* changes with script */
#define BMS_SCRIPT_WRCFGA_DATA_LENGTH               8u
#define WRCFGB_PEC_OFFSET                           14    /* This offset is only for a single BMS IC */
#define WRCFGB_DCC_OFFSET                           12    /* This offset is only for a single BMS IC */
#define WRCFGA_PEC_OFFSET                           6     /* This offset is only for a single BMS IC */
#define WRCFGA_DCC_OFFSET                           2     /* This offset is only for a single BMS IC */
#define BMS_CELLS_1_TO_2_MASK                       0x03
#else   /* Not supported */
#endif

#define BMS_SCRIPT_WRCFGB_DATA_LENGTH               8u
#define BMS_CMD_LENGTH                              4
#define CELL_BALANCE_DELAY_IN_SEC                   1 /* User can input an "activation delay", which is a time offset afer which cell balance command will be activated on the node */
#define ACTIVATION_DELAY                            ((CELL_BALANCE_DELAY_IN_SEC*1000)/2)
#define BMS_PAYLOAD_WRCFGB_OFFSET                   (BMS_PAYLOAD_EXCLUDE_WRCFGB_LENGTH+WRCFGB_DCC_OFFSET) /* This is for generic BMS scripts ADBMS6833*/
#define BMS_PAYLOAD_PACKET_ID_OFFSET                0
#define BMS_CELLS_1_TO_8_MASK                       0xFF
#define BMS_CELLS_9_TO_16_MASK                      0xFF00
#define DECIMAL_VAL_12_CELLS_SET                    4095
#define ASCII_CODE_NUM_0                            48
#define ASCII_CODE_NUM_9                            57
#define DECIMAL_BASE_NUMBER                         10
#define LEN_UART_RECEIVE_BUFF                       500
#define MAX_NUM_OF_NOTIF_BUFF_DCC_CHECK             10
#define MAX_NUM_OF_CELL_BALANCE_RETRY               5
#define MAC_ADDR_BYTE_6                             5
#define MAC_ADDR_BYTE_7                             MAC_ADDR_BYTE_6 + 1
#define MAC_ADDR_BYTE_8                             MAC_ADDR_BYTE_7 + 1
#define ACTIVATION_TIME_LENGTH                      2 /* 15 Bit length */
#define PACKET_HEADER_TIMESTAMP_OFFSET              1
/******************************************************************************
 * Enumerations
 *******************************************************************************/


/*******************************************************************************
 * Structures
 *******************************************************************************/
/**
 * @brief Script change data structure
 */
typedef struct
{
    uint8_t iCount;
    uint8_t iDeviceList[ADI_WIL_MAX_NODES * ADI_WIL_MAC_ADDR_SIZE];
    bool bFailureFlag[ADI_WIL_MAX_NODES];
    bool bCellBalanceSuccessFlag[ADI_WIL_MAX_NODES];
    uint8_t iRetryCount[ADI_WIL_MAX_NODES];
    adi_wil_script_change_t iDeviceChangeScriptInfo[ADI_WIL_MAX_NODES];
}adi_wil_app_cell_balance_change_t;

/*******************************************************************************/
/* Embedded WIL Wrapper Function Declarations                                                        */
/*******************************************************************************/
/**
 * @brief    WIL set script entry on the node wrapper function
 *
 * @details  Wrapper function will call WIL ModifyScript function to change specific
 *                   entries in the BMS script, currently utilsed for cell balancing operation.
 *
 * @param    pPack[in,out]              pointer to pack instance
 *
 * @return   bool        Indication of success or failure.
 */
bool adi_wil_example_ExecuteModifyScript(adi_wil_pack_t * const pPack, uint8_t CB_OPTION);

#endif /* _ADI_WIL_EXAMPLE_CELL_BALANCE_H */
