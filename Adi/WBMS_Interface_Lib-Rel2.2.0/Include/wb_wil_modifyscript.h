/******************************************************************************
 * @file    wb_wil_modifyscript.h
 *
 * @brief   State machine to set BMS/PMS script entry on node/manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_MODIFYSCRIPT_H
#define WB_WIL_MODIFYSCRIPT_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_ModifyScriptAPI (adi_wil_pack_internals_t * const pInternals,
                                     uint64_t iDeviceId,
                                     adi_wil_sensor_id_t eSensorId,
                                     adi_wil_script_change_t const * const pData);

void wb_wil_HandleModifyScriptResponse (adi_wil_pack_internals_t * const pInternals,
                                        uint64_t iDeviceId,
                                        wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_MODIFYSCRIPT_H
