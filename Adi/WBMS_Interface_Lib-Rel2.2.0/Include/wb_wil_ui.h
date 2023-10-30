/******************************************************************************
 * @file    wb_wil_ui.h
 *
 * @brief   Interface for non-safety modules to perform callbacks and events
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_UI_H
#define WB_WIL_UI_H

#include "adi_wil_types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

void wb_wil_ui_GenerateCb (adi_wil_pack_t const * const pPack,
                           adi_wil_api_t eNonSafetyAPI,
                           adi_wil_err_t rc,
                           void const * const pData);

void wb_wil_ui_GeneratePortCb (adi_wil_port_t const * const pPort,
                               adi_wil_api_t eNonSafetyAPI,
                               adi_wil_err_t rc,
                               void const * const pData);

void wb_wil_ui_GenerateEvent (adi_wil_pack_t const * const pPack,
                              adi_wil_event_id_t eNonSafetyEvent,
                              void const * const pData);

adi_wil_err_t wb_wil_ui_AcquireLock (adi_wil_pack_t const * const pPack,
                                     void const * const pLockID);

void wb_wil_ui_ReleaseLock (adi_wil_pack_t const * const pPack,
                            void const * const pLockID);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_UI_H
