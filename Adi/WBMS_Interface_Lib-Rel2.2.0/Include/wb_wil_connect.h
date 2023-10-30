/******************************************************************************
 * @file    wb_wil_connect.h
 *
 * @brief   Connect to network managers in a pack
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_CONNECT_H
#define WB_WIL_CONNECT_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_ConnectAPI (adi_wil_pack_t const * const pPack,
                                 adi_wil_pack_internals_t * const pInternals,
                                 adi_wil_port_t * const pManager0Port,
                                 adi_wil_port_t * const pManager1Port);

void wb_wil_HandleConnectResponse (adi_wil_pack_internals_t * const pInternals,
                                   uint64_t iDeviceId,
                                   wbms_cmd_resp_connect_t const * const pResponse);

void adi_wil_Reconnect (adi_wil_pack_internals_t * const pInternals);

void wb_wil_TriggerConnectRequest (adi_wil_pack_internals_t const * const pInternals,
                                   adi_wil_port_t * pPort);

void wb_wil_CheckConnectionTimeout (adi_wil_pack_internals_t * const pInternals,
                                    uint32_t iCurrentTime);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_CONNECT_H
