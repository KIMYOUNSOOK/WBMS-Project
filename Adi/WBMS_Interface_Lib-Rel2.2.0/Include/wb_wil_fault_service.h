/******************************************************************************
 * @file    wb_wil_env_monitoring_test.h
 *
 * @brief   Begin Environmental Monitoring Test on a pack
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_FAULT_SERVICE__H
#define WB_WIL_FAULT_SERVICE__H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/
adi_wil_err_t wb_wil_EnableFaultServicingAPI (adi_wil_pack_internals_t * const pInternals,
                                              uint64_t iDeviceId,
                                              bool bEnable);

void wb_wil_HandleFaultServicingResponse(adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_FAULT_SERVICE__H
