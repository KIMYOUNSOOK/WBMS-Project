/******************************************************************************
 * @file    wb_wil_get_contextual.h
 *
 * @brief   Get the Contextual Data from a Manager or Node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_GET_CONTEXTUAL_H
#define WB_WIL_GET_CONTEXTUAL_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_GetContextualDataAPI (adi_wil_pack_internals_t * const pInternals,
                                           uint64_t iDeviceId,
                                           adi_wil_contextual_id_t eContextualDataId);

void wb_wil_HandleGetContextualResponse (adi_wil_pack_internals_t * const pInternals,
                                         uint64_t iDeviceId,
                                         wbms_cmd_resp_get_contextual_t const * const pResponse,
                                         void const * const pData);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_GET_CONTEXTUAL_H
