/******************************************************************************
* @file    wb_wil_configure_cell_balancing.h
*
* @brief   ConfigureCellBalancing internal interface
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_WIL_CONFIGURE_CELL_BALANCING__H
#define WB_WIL_CONFIGURE_CELL_BALANCING__H

#include "adi_wil_types.h"
#include "adi_wil_safety_internals.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_ConfigureCellBalancingAPI (adi_wil_safety_internals_t * const pInternals,
                                                uint64_t iDeviceId,
                                                adi_wil_ddc_t const * const pDischargeDutyCycle,
                                                uint16_t iDuration,
                                                uint32_t iUVThreshold);

void wb_wil_HandleConfigureCellBalancingResponse (adi_wil_safety_internals_t * const pInternals,
                                                  uint8_t iDeviceId,
                                                  adi_wil_err_t rc);

void wb_wil_TimeoutConfigureCellBalancing (adi_wil_safety_internals_t const * const pInternals);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_CONFIGURE_CELL_BALANCING__H
