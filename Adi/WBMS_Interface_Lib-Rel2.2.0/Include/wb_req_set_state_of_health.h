/******************************************************************************
 * @file     wb_req_state_of_health_set.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_STATE_OF_HEALTH_SET_H
#define WB_REQ_STATE_OF_HEALTH_SET_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_set_state_of_health_t
 */
struct wbms_cmd_req_set_state_of_health_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iPercentage;           /* Integer representation of battery health. Value from 0 to 100 */
};

#endif //WB_REQ_STATE_OF_HEALTH_SET_H
