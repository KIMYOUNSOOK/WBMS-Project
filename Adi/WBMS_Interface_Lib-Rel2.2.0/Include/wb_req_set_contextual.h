/******************************************************************************
 * @file     wb_req_set_contextual.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_SET_CONTEXTUAL_H
#define WB_REQ_SET_CONTEXTUAL_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_set_contextual_t
 */
struct wbms_cmd_req_set_contextual_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iContextualId;         /* ID of contextual data */
    uint8_t iLength;               /* Length of the data that follows this header */
};

#endif //WB_REQ_SET_CONTEXTUAL_H
