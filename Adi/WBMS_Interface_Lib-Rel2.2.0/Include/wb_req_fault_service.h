/******************************************************************************
 * @file     wb_req_fault_service.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_FAULT_SERVICE_H
#define WB_REQ_FAULT_SERVICE_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_fault_service_t
 */
struct wbms_cmd_req_fault_service_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iEnable;               /* Flag to enable/disable fault servicing (1 to enable) */
};

#endif // WB_REQ_FAULT_SERVICE_H
