/******************************************************************************
 * @file     wb_req_otap_hs.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_OTAP_HS_H
#define WB_REQ_OTAP_HS_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_cmd_req_otap_handshake_t
 */
struct wbms_cmd_req_otap_handshake_t {
    uint16_t iToken;                                         /* Token to match request with response */
    uint8_t iFileType;                                       /* Type of file being loaded */
    uint8_t iHeaderData[WB_WIL_OTAP_FILE_HEADER_LEN];        /* File header to be processed on the manager */
};

#endif //WB_REQ_OTAP_HS_H
