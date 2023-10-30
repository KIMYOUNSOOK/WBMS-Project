/******************************************************************************
* @file    wb_assl_fusa.h
*
* @brief   WIL FE Application Specific Safety Layer (ASSL) Safety Component Interface
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_ASSL_FUSA__H
#define WB_ASSL_FUSA__H

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

void wb_assl_Process (adi_wil_safety_internals_t * const pInternals,
                      uint32_t iCurrentTicks);

adi_wil_err_t wb_assl_Initialize (adi_wil_pack_t const * const pPack,
                                  adi_wil_sensor_data_t * const pDataBuffer,
                                  uint16_t iDataBufferCount);

adi_wil_err_t wb_assl_WriteConfigureCellBalancingFrame (adi_wil_safety_internals_t * const pInternals,
                                                        uint8_t iDeviceId,
                                                        adi_wil_ddc_t const * const pDischargeDutyCycle,
                                                        uint16_t iDuration,
                                                        uint32_t iUVThreshold);

adi_wil_err_t wb_assl_WriteGetCellBalancingStatusFrame (adi_wil_safety_internals_t * const pInternals);

adi_wil_err_t wb_assl_ActivateRequest (adi_wil_safety_internals_t * const pInternals,
                                       adi_wil_api_t eAPI,
                                       uint8_t iRetries,
                                       uint32_t iRetryInterval);

void wb_assl_DeactivateRequest (adi_wil_safety_internals_t * const pInternals);

void wb_assl_HandleValidatedSCLFrame (adi_wil_safety_internals_t * const pInternals,
                                      wb_msg_header_t * const pMsgHeader,
                                      uint8_t const * const pData);

void wb_assl_ReportValidationError (adi_wil_pack_t const * const pPack);

#ifdef __cplusplus
}
#endif
#endif  //WB_ASSL_FUSA__H
