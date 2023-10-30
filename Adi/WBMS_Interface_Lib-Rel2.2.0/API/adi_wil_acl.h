/******************************************************************************
 * @file     adi_wil_acl.h
 *
 * @brief    Access control list type definition
 *
 * @details  Contains access control list data type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_ACL__H
#define ADI_WIL_ACL__H

#include <stdint.h>
#include "adi_wil_types.h"
/**
 * @brief   Access control list structure
 */
struct adi_wil_acl_t {
    uint8_t Data[ADI_WIL_MAX_NODES * ADI_WIL_MAC_ADDR_SIZE];	/*!< Buffer of 8-byte MAC addresses */
    uint8_t iCount;                                             /*!< Number of 8-byte MAC addresses in the list */
};

#endif // ADI_WIL_ACL__H
