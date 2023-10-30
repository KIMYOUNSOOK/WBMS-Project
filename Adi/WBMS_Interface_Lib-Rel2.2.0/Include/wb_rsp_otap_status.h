/******************************************************************************
 * @file     wb_rsp_otap_status.h
 *
 * @brief
 *
 * @details
 *           This header is used by the WBMS Interface Library.
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_OTAP_STATUS_H
#define WB_RSP_OTAP_STATUS_H

#include <stdint.h>
#include "wbms_cmd_defs.h"

/**
 * @brief   wbms_cmd_resp_otap_status_t
 */
struct wbms_cmd_resp_otap_status_t {
    uint16_t iToken;                                            /* Token to match request with response */
    uint16_t iIndex;                                            /* Current base index of file transfer sector */
    uint8_t  MissingBlocks[WBMS_OTAP_MISSING_BLOCK_MASK_LEN];   /* Mask of missing blocks from this base index */
    uint8_t  rc;                                                /* RC to indicate whether the above values are valid or if the device is aborting the OTAP process */
};

#endif //WB_RSP_OTAP_STATUS_H
