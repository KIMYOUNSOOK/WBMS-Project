/******************************************************************************
* @file    wb_xms_fusa.h
*
* @brief   Safety interface for XMS buffering
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_XMS_FUSA__H
#define WB_XMS_FUSA__H

#include "adi_wil_types.h"
#include "adi_wil_safety_internals.h"
#include "wb_wil_xms_type.h"
#include "wb_wil_msg_header.h"
#include "adi_wil_xms_parameters.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_xms_Initialize (adi_wil_safety_internals_t * const pInternals,
                                 adi_wil_sensor_data_t * const pDataBuffer,
                                 uint16_t iDataBufferCount);

adi_wil_err_t wb_xms_CheckTimeouts (adi_wil_safety_internals_t * const pInternals,
                                    uint32_t iCurrentTicks);

adi_wil_err_t wb_xms_HandleFuSaMeasurement (adi_wil_safety_internals_t * const pInternals,
                                            wb_msg_header_t const * const pMsgHeader,
                                            wb_xms_metadata_t const * const pXmsMetadata,
                                            uint8_t const * const pData);

void wb_xms_InitializeAllocation (adi_wil_safety_internals_t * const pInternals,
                                  uint16_t iPMSPackets,
                                  uint16_t iEMSPackets);

void wb_xms_UpdateXMSAllocation (adi_wil_safety_internals_t * const pInternals,
                                 bool bFuSaContext);

#ifdef __cplusplus
}
#endif
#endif  //WB_XMS_FUSA__H
