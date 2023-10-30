/******************************************************************************
 * @file     wb_req_get_file_crc.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_FILE_CRC_GET_H
#define WB_REQ_FILE_CRC_GET_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_cmd_req_get_file_crc_t
 */
struct wbms_cmd_req_get_file_crc_t {
    uint16_t iToken;               /* Token to match response with request */
    uint8_t iFileType;             /* File type to retrieve CRC for */    
};

#endif //WB_REQ_FILE_CRC_GET_H
