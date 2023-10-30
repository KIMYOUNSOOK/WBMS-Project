/******************************************************************************
 * @file     wb_wil_msg_header.h
 *
 * @brief    WIL FE frame header structure definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_MSG_HEADER_H
#define WB_WIL_MSG_HEADER_H

#include "adi_wil_types.h"
#include <stdint.h>

/******************************************************************************
 * Structure Definitions
 *****************************************************************************/

/**
 * @brief   Frame header structure
 */
typedef struct
{
    uint8_t iSourceDeviceId;
    uint8_t iSequenceNumber;
    uint8_t iMessageType;
    uint8_t iPayloadLength;
} wb_msg_header_t;

#endif //WB_WIL_MSG_HEADER_H
