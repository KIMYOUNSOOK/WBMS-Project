/******************************************************************************
 * @file     wb_req_get_file.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_GET_FILE_H
#define WB_REQ_GET_FILE_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_get_file_t
 */
struct wbms_cmd_req_get_file_t {
    uint16_t iToken;              /* Token to match request with response */
    uint16_t iOffset;             /* Offset in log file to retrieve */
    uint8_t iFileType;            /* File type to retrieve */
};

#endif //WB_REQ_GET_FILE_H
