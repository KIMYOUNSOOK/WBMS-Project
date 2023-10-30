/*******************************************************************************
 * @file    wb_wil_resetdevice.h
 *
 * @brief   Highest level (WIL) State Machine for resetting a Network manager
 *          or Node
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
******************************************************************************/

#ifndef WB_WIL_RESETDEVICE_H
#define WB_WIL_RESETDEVICE_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_ResetDeviceAPI (adi_wil_pack_internals_t * const pInternals,
                                     uint64_t iDeviceId);

adi_wil_err_t wb_wil_ResetMgrNotifyConnectionLost (adi_wil_pack_internals_t * const pInternals,
                                                   uint64_t iDeviceId);

void wb_wil_HandleResetResponse (adi_wil_pack_internals_t * const pInternals,
                                 uint64_t iDeviceId,
                                 wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_RESETDEVICE_H
