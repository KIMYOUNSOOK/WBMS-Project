/******************************************************************************
 * @file     wb_rsp_generic.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_GENERIC_H
#define WB_RSP_GENERIC_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_resp_generic_t
 */
struct wbms_cmd_resp_generic_t {
    uint16_t iToken;               /* Token to match response with request*/
    uint8_t rc;                    /* RC indicating if the response was successful */
};

#endif //WB_RSP_GENERIC_H
