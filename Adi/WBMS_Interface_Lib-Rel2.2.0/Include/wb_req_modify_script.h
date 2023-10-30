/******************************************************************************
 * @file     wb_req_script_modify.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_SCRIPT_MODIFY_H
#define WB_REQ_SCRIPT_MODIFY_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_modify_script_t
 */
struct wbms_cmd_req_modify_script_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iSensorId;             /* Sensor ID on the device running the script to modify */
    uint16_t iActivationTime;      /* Timestamp of when the script modification should occur */
    uint16_t iOffset;              /* Offset in the script to modify */
    uint8_t  iLength;              /* Length of the data that follows this header */
    uint16_t iCRC;                 /* CRC to cover the header fields from iSensorId to iLength */
};

#endif //WB_REQ_SCRIPT_MODIFY_H
