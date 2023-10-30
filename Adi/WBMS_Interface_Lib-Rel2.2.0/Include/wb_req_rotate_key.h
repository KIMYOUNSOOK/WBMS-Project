/******************************************************************************
 * @file     wb_cmd_req_rotate_key.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_CMD_REQ_ROTATE_KEY_H
#define WB_CMD_REQ_ROTATE_KEY_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_rotate_key_t
 */
struct wbms_cmd_req_rotate_key_t {
    uint16_t iToken;                        /* Token to match request with response */
    uint8_t iDeviceId;                      /* Index in ACL of device to rotate key for */
};

#endif //WB_CMD_REQ_ROTATE_KEY_H
