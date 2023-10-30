/******************************************************************************
 * @file    wb_wil_set_contextual.h
 *
 * @brief   Get the device software version on a Manager or Node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_GET_VERSION_H
#define WB_WIL_GET_VERSION_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_GetWILSoftwareVersionAPI (adi_wil_version_t * const pVersion);

adi_wil_err_t wb_wil_GetDeviceVersionAPI (adi_wil_pack_internals_t * const pInternals,
                                          uint64_t iDeviceId);

void wb_wil_HandleGetVersionResponse (adi_wil_pack_internals_t * const pInternals,
                                      uint64_t iDeviceId,
                                      wbms_cmd_resp_get_version_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_GET_VERSION_H
