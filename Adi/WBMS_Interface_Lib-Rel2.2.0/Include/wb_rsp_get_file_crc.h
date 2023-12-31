/******************************************************************************
 * @file     wb_rsp_file_crc_get.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_GET_FILE_CRC_H
#define WB_RSP_GET_FILE_CRC_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_cmd_resp_get_file_crc_t
 */
struct wbms_cmd_resp_get_file_crc_t {
    uint16_t iToken;               /* Token to match response with request */
    uint32_t iCRC;                 /* CRC of requested file generated by the device */
    uint8_t rc;                    /* Return code indicating if the above values are valid */    
};

#endif //WB_RSP_GET_FILE_CRC_H
