/******************************************************************************
 * @file     wb_rsp_get_mon_params_crc.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_GET_MON_PARAMS_CRC_H
#define WB_RSP_GET_MON_PARAMS_CRC_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_cmd_resp_get_mon_params_crc_t
 */
struct wbms_cmd_resp_get_mon_params_crc_t {
    uint16_t iToken;               /* Token to match response with request */
    uint32_t iCRC;                 /* CRC of the monitor parameter table */
    uint8_t rc;                    /* RC indicating if the above values are valid */
};

#endif //WB_RSP_GET_MON_PARAMS_CRC_H
