/******************************************************************************
 * @file     wb_rsp_contextual_get.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_CONTEXTUAL_GET_H
#define WB_RSP_CONTEXTUAL_GET_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_resp_get_contextual_t
 */
struct wbms_cmd_resp_get_contextual_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iLength;               /* Length of the data that follows this header */
    uint8_t rc;                    /* RC indicating if the above values are valid */
};

#endif //WB_RSP_CONTEXTUAL_GET_H
