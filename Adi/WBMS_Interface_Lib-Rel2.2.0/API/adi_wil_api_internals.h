/******************************************************************************
 * @file     adi_wil_api_internals.h
 *
 * @brief    WIL FE API internals structure definitions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_API_INTERNALS__H
#define ADI_WIL_API_INTERNALS__H

#include "adi_wil_types.h"
#include <stdint.h>

/******************************************************************************
 * Structure Definitions
 *****************************************************************************/

/**
 * @brief   Structure for storing parameters used by API modules
 */
typedef struct
{
    uint64_t iRequestedNodes;       /*!< Bitmap of all nodes in the API request */
} adi_wil_api_internals_t;

#endif //ADI_WIL_API_INTERNALS__H
