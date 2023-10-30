/******************************************************************************
 * @file     adi_wil_connection_details.h
 *
 * @brief    WBMS Connection status definitions
 *
 * @details  Contains Connection details type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_CONNECTION_DETAILS_H
#define ADI_WIL_CONNECTION_DETAILS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief   Connection details structure
 */
struct adi_wil_connection_details_t {
    uint32_t iHash;         /*!< Hash of the current configuration file loaded to the network managers. Only valid if both managers are loaded with the same configuration. */
    bool bDMHActive;        /*!< Boolean value indicating if either of the network managers are currently operating with a DMH topology */
};

#endif //ADI_WIL_CONNECTION_DETAILS_H
