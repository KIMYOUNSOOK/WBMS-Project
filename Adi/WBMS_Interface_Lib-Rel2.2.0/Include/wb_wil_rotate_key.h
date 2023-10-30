/******************************************************************************
 * @file    wb_wil_rotate_key.h
 *
 * @brief   Rotate the join key on one or all nodes
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_ROTATE_KEY_H
#define WB_WIL_ROTATE_KEY_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_RotateKeyAPI (adi_wil_pack_internals_t * const pInternals,
                                   adi_wil_key_t eKeyType,
                                   uint64_t iDeviceId);

void wb_wil_HandleRotateKeyResponse (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_ROTATE_KEY_H
