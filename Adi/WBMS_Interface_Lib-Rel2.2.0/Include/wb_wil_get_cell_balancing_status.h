/******************************************************************************
* @file    wb_wil_configure_cell_balancing.h
*
* @brief   GetCellBalancingStatus internal interface
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_WIL_GET_CELL_BALANCING_STATUS__H
#define WB_WIL_GET_CELL_BALANCING_STATUS__H

#include "adi_wil_types.h"
#include "adi_wil_safety_internals.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetCellBalancingStatusAPI (adi_wil_safety_internals_t * const pInternals);

void wb_wil_HandleGetCellBalancingStatusResponse (adi_wil_safety_internals_t * const pInternals,
                                                  adi_wil_cell_balancing_status_t const * const pStatus,
                                                  adi_wil_err_t rc);

void wb_wil_TimeoutGetCellBalancingStatus (adi_wil_safety_internals_t const * const pInternals);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_GET_CELL_BALANCING_STATUS__H
