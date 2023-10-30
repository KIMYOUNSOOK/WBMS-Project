/******************************************************************************
 * @file     wb_rsp_get_file.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_CMD_RSP_GET_FILE_H
#define WB_CMD_RSP_GET_FILE_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_resp_get_file_t
 */
struct wbms_cmd_resp_get_file_t {
    uint16_t iToken;                  /* Token to match request with response */
    uint16_t iRemainingBytes;         /* Remaining bytes in log file after this block */
    uint8_t iLength;                  /* Number of bytes to follow this header */
    uint8_t rc;                       /* RC indicating if the above values are valid */
};

#endif //WB_CMD_RSP_GET_FILE_H
