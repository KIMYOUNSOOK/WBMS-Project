/******************************************************************************
 * @file     wb_req_set_mon_params.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_SET_MON_PARAMS_DATA_H
#define WB_REQ_SET_MON_PARAMS_DATA_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_set_mon_params_data_t
 */
struct wbms_cmd_req_set_mon_params_data_t {
    uint16_t iToken;               /* Token to match request with response */
    uint16_t iOffset;              /* Offset in the table to modify */
    uint16_t iCRC;                 /* CRC of the data */
    uint8_t iLength;               /* Length of the data that follows this header */
};

#endif //WB_REQ_SET_MON_PARAMS_DATA_H
