/******************************************************************************
 * @file     wb_req_connectl.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_CONNECT_H
#define WB_REQ_CONNECT_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_get_acl_t
 */
struct wbms_cmd_req_connect_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iDeviceId;             /* iDeviceId that the recieving device will be referred to in configurations */
};

#endif //WB_REQ_CONNECT_H
