/******************************************************************************
 * @file     adi_wil_contextual_data.h
 *
 * @brief    WBMS contextual data definitions
 *
 * @details  Contains contextual data type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_CONTEXTUAL_DATA__H
#define ADI_WIL_CONTEXTUAL_DATA__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Contextual Data Rx Buffer
 */
struct adi_wil_contextual_data_t {
    uint8_t iLength;                             /*!< number of valid bytes in the buffer Data */
    uint8_t Data[ADI_WIL_CONTEXTUAL_DATA_SIZE];  /*!< array to hold contextual data */
};

#endif //ADI_WIL_CONTEXTUAL_DATA__H
