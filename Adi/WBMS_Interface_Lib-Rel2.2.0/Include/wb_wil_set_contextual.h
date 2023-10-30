/******************************************************************************
 * @file    wb_wil_set_contextual.h
 *
 * @brief   Set the Contextual Data on a Manager or Node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_SET_CONTEXTUAL_H
#define WB_WIL_SET_CONTEXTUAL_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_SetContextualDataAPI (adi_wil_pack_internals_t * const pInternals,
                                           uint64_t iDeviceId,
                                           adi_wil_contextual_id_t eContextualDataId,
                                           adi_wil_contextual_data_t const * const pData);

void wb_wil_HandleSetContextualResponse (adi_wil_pack_internals_t * const pInternals,
                                         uint64_t iDeviceId,
                                         wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_SET_CONTEXTUAL_H
