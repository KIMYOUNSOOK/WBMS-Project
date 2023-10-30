/******************************************************************************
 * @file    wb_wil_inventory_transition.h
 *
 * @brief   Enter or exit inventory state on one or all nodes
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_INVENTORY_TRANSITION_H
#define WB_WIL_INVENTORY_TRANSITION_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_EnterInventoryStateAPI (adi_wil_pack_internals_t * const pInternals,
                                             uint64_t iDeviceId,
                                             bool bEnable,
                                             uint64_t iCurrentTime);

void wb_wil_HandleInventoryTransitionResponse(adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_INVENTORY_TRANSITION_H
