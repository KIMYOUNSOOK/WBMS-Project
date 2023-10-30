/******************************************************************************
 * @file    wb_wil_update_monitor_params.h
 *
 * @brief   Updates the monitor parameters on one or more devices
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_UPDATE_MONITOR_PARAMS_H
#define WB_WIL_UPDATE_MONITOR_PARAMS_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"
#include "wb_rsp_generic.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_UpdateMonitorParametersAPI (adi_wil_pack_internals_t * const pInternals,
                                                 uint64_t iDeviceId,
                                                 uint8_t * const pData,
                                                 uint16_t iLength);

void wb_wil_HandleUpdateMonParamDataResponse (adi_wil_pack_internals_t * const pInternals,
                                              uint64_t iDeviceId,
                                              wbms_cmd_resp_generic_t const * const pResponse);

void wb_wil_HandleUpdateMonParamCommitResponse (adi_wil_pack_internals_t * const pInternals,
                                                uint64_t iDeviceId,
                                                wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_UPDATE_MONITOR_PARAMS_H
