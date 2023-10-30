/******************************************************************************
* @file    wb_xms.h
*
* @brief   Non-safety interface for XMS buffering
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_XMS__H
#define WB_XMS__H

#include "adi_wil_types.h"
#include "wb_wil_xms_type.h"
#include "wb_wil_msg_header.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_xms_HandleMeasurement (adi_wil_pack_t const * const pPack,
                                        wb_msg_header_t const * const pMsgHeader,
                                        wb_xms_metadata_t const * const pXmsMetadata,
                                        uint8_t const * const pData);

void wb_xms_Flush (adi_wil_pack_t const * const pPack,
                   adi_wil_xms_type_t eType);

#ifdef __cplusplus
}
#endif
#endif  //WB_XMS__H
