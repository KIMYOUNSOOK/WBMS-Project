/******************************************************************************
 * @file     wb_req_get_contextual.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_GET_CONTEXTUAL_H
#define WB_REQ_GET_CONTEXTUAL_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_get_contextual_t
 */
struct wbms_cmd_req_get_contextual_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iContextualId;         /* ID of contextual data */
};

#endif //WB_REQ_SET_CONTEXTUAL_H
