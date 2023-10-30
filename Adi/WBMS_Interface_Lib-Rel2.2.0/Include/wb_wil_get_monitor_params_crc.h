/******************************************************************************
 * @file    wb_wil_get_monitor_params_crc.h
 *
 * @brief   Retrieves the CRC of the monitor parameters from a devices
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_GET_MONITOR_PARAMS_CRC_H
#define WB_WIL_GET_MONITOR_PARAMS_CRC_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"
#include "wb_rsp_get_mon_params_crc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_GetMonitorParametersCRCAPI (adi_wil_pack_internals_t * const pInternals,
                                                 uint64_t iDeviceId);

void wb_wil_HandleGetMonitorParametersCRCResponse (adi_wil_pack_internals_t * const pInternals,
                                                   wbms_cmd_resp_get_mon_params_crc_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_GET_MONITOR_PARAMS_CRC_H
