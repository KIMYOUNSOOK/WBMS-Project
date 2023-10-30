/******************************************************************************
 * @file     wb_rsp_version_get.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_VERSION_GET_H
#define WB_RSP_VERSION_GET_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_cmd_resp_get_version_t
 */
struct wbms_cmd_resp_get_version_t {
    uint16_t iToken;               /* Token to match request with response */
    uint16_t iVersionMajor;        /* Main processor SW major revision */
    uint16_t iVersionMinor;        /* Main processor SW minor revision */
    uint16_t iVersionPatch;        /* Main processor SW patch revision */
    uint16_t iVersionBuild;        /* Main processor SW build revision */
    uint16_t iSiliconVersion;      /* Main processor Silicon revision */
    uint16_t iCPVersionMajor;      /* Co processor SW major revision */
    uint16_t iCPVersionMinor;      /* Co processor SW minor revision */
    uint16_t iCPVersionPatch;      /* Co processor SW patch revision */
    uint16_t iCPVersionBuild;      /* Co processor SW build revision */
    uint16_t iCPSiliconVersion;    /* Co processor Silicon revision */
    uint32_t iLifeCycleInfo;       /* Life cycle information */
    uint8_t rc;                    /* RC indicating if the above values are valid */
};

#endif //WB_RSP_VERSION_GET_H
