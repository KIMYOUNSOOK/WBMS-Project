/******************************************************************************
 * @file     wb_req_generic.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_GENERIC_H
#define WB_REQ_GENERIC_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_generic_t
 */
struct wbms_cmd_req_generic_t {
    uint16_t iToken;               /* Token to match request with response */
};

#endif //WB_REQ_GENERIC_H
