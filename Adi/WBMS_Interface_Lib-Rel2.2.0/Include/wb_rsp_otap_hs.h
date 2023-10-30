/******************************************************************************
 * @file     wb_rsp_otap_hs.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_OTAP_HS_H
#define WB_RSP_OTAP_HS_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_resp_otap_handshake_t
 */
struct wbms_cmd_resp_otap_handshake_t {
    uint16_t iToken;               /* Token to match request with response */
    uint32_t iFileSize;            /* Total file size extracted from file header */
    uint8_t rc;                    /* RC to indicate if the above values are valid and if the device accepts this file */
};

#endif //WB_RSP_OTAP_HS_H
