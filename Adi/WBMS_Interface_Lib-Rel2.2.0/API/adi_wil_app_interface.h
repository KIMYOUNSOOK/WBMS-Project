/******************************************************************************
 * @file     adi_wil_app_interface.h
 *
 * @brief    WBMS Interface Library application interface declarations.
 *
 * @details  Contains API declarations for the application interface functions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_APP_INTERFACE__H
#define ADI_WIL_APP_INTERFACE__H

#include "adi_wil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Function Declarations
 *****************************************************************************/

void adi_wil_HandleCallback (adi_wil_pack_t const * const pPack,
                             void const * const pClientData,
                             adi_wil_api_t eAPI,
                             adi_wil_err_t rc,
                             void const * const pData);
 
void adi_wil_HandlePortCallback (adi_wil_port_t const * const pPort,
                                 adi_wil_api_t eAPI,
                                 adi_wil_err_t rc,
                                 void const * const pData);

void adi_wil_HandleEvent (adi_wil_pack_t const * const pPack,
                          void const * const pClientData,
                          adi_wil_event_id_t EventCode,
                          void const * const pData);

#ifdef __cplusplus
}
#endif
#endif   // #ifndef ADI_WIL_APP_INTERFACE__H
