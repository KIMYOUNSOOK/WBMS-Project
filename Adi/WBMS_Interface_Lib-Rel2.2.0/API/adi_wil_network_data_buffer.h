/******************************************************************************
 * @file     adi_wil_network_data_buffer.h
 *
 * @brief    WBMS network data buffer definitions
 *
 * @details  Contains network data buffer type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_NETWORK_DATA_BUFFER_H
#define ADI_WIL_NETWORK_DATA_BUFFER_H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Network data buffer
 */
struct adi_wil_network_data_buffer_t {
    adi_wil_network_data_t * pData;             /*!< Pointer to network data array */
    uint16_t iDataBufferCount;                  /*!< Total entries in the data array */
    uint16_t iCount;                            /*!< Number of entries in the array */
};

#endif // ADI_WIL_NETWORK_DATA_BUFFER_H
