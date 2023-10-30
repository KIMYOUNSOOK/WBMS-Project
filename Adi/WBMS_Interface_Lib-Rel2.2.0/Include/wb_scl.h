/******************************************************************************
* @file    wb_scl.h
*
* @brief   WIL FE Safety Communications Layer non-safety component interface
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_SCL__H
#define WB_SCL__H

#include "adi_wil_types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

void wb_scl_HandleSCLFrame (adi_wil_pack_t const * const pPack,
                            uint8_t iDeviceId,
                            uint8_t iLength,
                            uint8_t const * const pData);

#ifdef __cplusplus
}
#endif
#endif  //WB_SCL__H
