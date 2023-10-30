/******************************************************************************
 * @file    wb_wil_api.h
 *
 * @brief   Functions required to setup, validate and log an API invocation
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_API_H
#define WB_WIL_API_H

#include "adi_wil_types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t adi_wil_ValidateInstance (adi_wil_pack_internals_t const * const pInternals, bool bCheckConnected);

adi_wil_err_t adi_wil_CheckNullableParams (uint8_t iParamCount, void const * const Params[]);

adi_wil_err_t adi_wil_CheckSystemMode (adi_wil_pack_internals_t const * const pInternals, uint8_t iModeCount, adi_wil_mode_t const ValidModes[]);

adi_wil_err_t adi_wil_CheckFileType (adi_wil_pack_internals_t const * const pInternals, adi_wil_file_type_t eFileType, uint8_t iCount, adi_wil_file_type_t const ValidTypes[]);

void wb_wil_api_CheckForTimeout (adi_wil_pack_internals_t * const pInternals, uint32_t iCurrentTime);

adi_wil_err_t wb_wil_api_StartTimer (adi_wil_pack_internals_t * const pInternals, uint16_t * const pToken, uint32_t iTimeout);

adi_wil_err_t wb_wil_api_CheckToken (adi_wil_pack_internals_t * const pInternals, uint16_t iToken, bool bClear);

#ifdef __cplusplus
}
#endif
#endif // #ifndef WB_WIL_API_H
