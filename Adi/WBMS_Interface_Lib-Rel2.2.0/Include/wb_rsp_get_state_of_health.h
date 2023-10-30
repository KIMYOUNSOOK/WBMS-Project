/******************************************************************************
 * @file     wb_rsp_state_of_health_get.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_STATE_OF_HEALTH_GET_H
#define WB_RSP_STATE_OF_HEALTH_GET_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_resp_get_state_of_health_t
 */
struct wbms_cmd_resp_get_state_of_health_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iPercentage;           /* Integer representation of battery health. Value from 0 to 100 */
    uint8_t rc;                    /* RC indicating if the above values are valid */
};

#endif //WB_RSP_STATE_OF_HEALTH_GET_H
