/******************************************************************************
 * @file    wb_wil_selectscript.h
 *
 * @brief   State machine to select BMS/PMS script id on node/manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_SELECTSCRIPT_H
#define WB_WIL_SELECTSCRIPT_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_SelectScriptAPI (adi_wil_pack_internals_t * const pInternals,
                                      uint64_t iDeviceId,
                                      adi_wil_sensor_id_t eSensorId,
                                      uint8_t iScriptId);

void wb_wil_HandleSelectScriptResponse (adi_wil_pack_internals_t * const pInternals,
                                        uint64_t iDeviceId,
                                        wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_SELECTSCRIPT_H
