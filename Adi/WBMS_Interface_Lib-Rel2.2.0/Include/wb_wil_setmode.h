/******************************************************************************
 * @file    wb_wil_setmode.h
 *
 * @brief   Changes the system mode of the WBMS network.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#ifndef WB_WIL_SETMODE_H
#define WB_WIL_SETMODE_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SetModeAPI (adi_wil_pack_internals_t * const pInternals,
                                 adi_wil_mode_t eMode);

adi_wil_err_t wb_wil_GetModeAPI (adi_wil_pack_internals_t const * const pInternals,
                                 adi_wil_mode_t * const pMode);

void wb_wil_HandleSetModeResponse (adi_wil_pack_internals_t * const pInternals,
                                   uint64_t iDeviceId,
                                   wbms_cmd_resp_set_mode_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_SETMODE_H
