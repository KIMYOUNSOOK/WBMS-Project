/******************************************************************************
* @file    wb_assl.h
*
* @brief   WIL FE Application Specific Safety Layer
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_ASSL__H
#define WB_ASSL__H

#include "adi_wil_types.h"
#include "adi_wil_safety_internals.h"
#include "wb_wil_msg_header.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

bool wb_assl_GetPendingMessage (adi_wil_pack_t const * const pPack,
                               uint8_t ** const pMessage,
                               uint8_t * const pDeviceId);

void wb_assl_ReleaseBuffer (adi_wil_pack_t const * const pPack,
                            uint8_t const * const pMessage);

void wb_assl_InitializeAllocation (adi_wil_pack_t const * const pPack,
                                   uint16_t iPMSPackets,
                                   uint16_t iEMSPackets);

void wb_assl_SetMeasurementParameters (adi_wil_pack_t const * const pPack,
                                       adi_wil_xms_parameters_t const * const pXMSParameters);

#ifdef __cplusplus
}
#endif
#endif  //WB_ASSL__H
