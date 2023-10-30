/******************************************************************************
 * @file     wb_req_script_select.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_SCRIPT_SELECT_H
#define WB_REQ_SCRIPT_SELECT_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_select_script_t
 */
struct wbms_cmd_req_select_script_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iSensorId;             /* Sensor ID on the device to select the script for */
    uint8_t iScriptId;             /* Script ID to select from container */
};

#endif //WB_REQ_SCRIPT_SELECT_H
