/******************************************************************************
* @file    wb_wil_xms_sequence_generation.h
*
* @brief   Non-FuSa XMS Sequence Number Generation via Timestamp
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_WIL_XMS_SEQUENCE_GENERATION_H
#define WB_WIL_XMS_SEQUENCE_GENERATION_H

#include "adi_wil_types.h"
#include "wb_wil_xms_type.h"
#include "wb_wil_msg_header.h"
#include <stdint.h>

void wb_wil_HandleXmsMeasurement (adi_wil_pack_internals_t * const pInternals,
                                  wb_msg_header_t * const pMsgHeader,
                                  wb_xms_metadata_t * const pXmsMetadata,
                                  uint8_t const * const pData,
                                  uint64_t iDeviceId);

void wb_wil_XmsStandbyTransition (adi_wil_pack_internals_t * const pInternals);

#endif  //WB_WIL_XMS_SEQUENCE_GENERATION_H
