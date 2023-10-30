/******************************************************************************
* @file    wb_scl_fusa.h
*
* @brief   WIL FE Safety Communications Layer safety component interface
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_SCL_FUSA__H
#define WB_SCL_FUSA__H

#include <stdint.h>
#include <stdbool.h>
#include "wb_wil_msg_header.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
 * Public functions
 *****************************************************************************/

bool wb_scl_WrapSCLFrame (wb_msg_header_t const * const pMsgHeader,
                          uint8_t * const pBuffer);

#ifdef __cplusplus
}
#endif
#endif  //WB_SCL_FUSA__H
