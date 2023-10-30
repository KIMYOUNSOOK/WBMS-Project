/******************************************************************************
 * @file    wb_wil_ui_fusa.h
 *
 * @brief   Interface for safety modules to perform callbacks and events
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_UI_FUSA_H
#define WB_WIL_UI_FUSA_H

#include "adi_wil_types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

void wb_wil_ui_GenerateFuSaCb (adi_wil_pack_t const * const pPack,
                               adi_wil_api_t eSafetyAPI,
                               adi_wil_err_t rc,
                               void const * const pData);

void wb_wil_ui_GenerateFuSaEvent (adi_wil_pack_t const * const pPack,
                                  adi_wil_event_id_t eSafetyEvent,
                                  void const * const pData);

adi_wil_err_t wb_wil_ui_AcquireFuSaLock (adi_wil_pack_t const * const pPack,
                                         void const * const pLockID);

void wb_wil_ui_ReleaseFuSaLock (adi_wil_pack_t const * const pPack,
                                void const * const pLockID);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_UI_FUSA_H
