/******************************************************************************
 * @file     wb_req_mode_set.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_MODE_SET_H
#define WB_REQ_MODE_SET_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_set_mode_t
 */
struct wbms_cmd_req_set_mode_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iMode;                 /* Mode of the WBMS to transition to */
};

#endif //WB_REQ_MODE_SET_H
