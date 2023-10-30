/******************************************************************************
 * @file     wb_req_data_send.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_DATA_SEND_H
#define WB_REQ_DATA_SEND_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_send_data_t
 */
struct wbms_cmd_req_send_data_t {
    uint16_t iToken;               /* Token to match response with request*/
    uint8_t iDeviceId;                  /* MAC to unicast message to. 0xFFFFFFFFFFFF to broadcast to all nodes */
    uint8_t iLength;               /* Length of the data following this header */
    uint8_t iHighPriority;         /* Flag to indicate if this request should be priorites over existing messages in the queue */
    uint8_t iPortId;                /* Destination port ID */
};

#endif //WB_REQ_DATA_SEND_H
