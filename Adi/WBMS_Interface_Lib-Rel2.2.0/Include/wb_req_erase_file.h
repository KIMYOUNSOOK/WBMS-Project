/******************************************************************************
 * @file     wb_req_erase_file.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_ERASE_FILE_H
#define WB_REQ_ERASE_FILE_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_erase_file_t
 */
struct wbms_cmd_req_erase_file_t {
    uint16_t iToken;               /* Token to match response with request */
    uint8_t iFileType;             /* File type to erase */
};

#endif //WB_REQ_ERASE_FILE_H
