/******************************************************************************
 * @file    wb_wil_get_state_of_health.h
 *
 * @brief   Get the State of health on a node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_GET_STATE_OF_HEALTH_H
#define WB_WIL_GET_STATE_OF_HEALTH_H

#include "adi_wil_types.h"
#include "wbms_cmd_node_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_GetStateOfHealthAPI (adi_wil_pack_internals_t * const pInternals,
                                          uint64_t iDeviceId);

void wb_wil_HandleGetStateOfHealthResponse (adi_wil_pack_internals_t * const pInternals,
                                            uint64_t iDeviceId,
                                            wbms_cmd_resp_get_state_of_health_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_GET_STATE_OF_HEALTH_H
