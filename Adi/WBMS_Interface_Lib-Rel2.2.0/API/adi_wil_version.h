/******************************************************************************
 * @file     adi_wil_version.h
 *
 * @brief    Software version type definition
 *
 * @details  Contains Software version data type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_VERSION__H
#define ADI_WIL_VERSION__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Software version information structure
 */
struct adi_wil_version_t {
    uint16_t iVersionMajor;	        /*!< Major version number */
    uint16_t iVersionMinor;	        /*!< Minor version number */
    uint16_t iVersionPatch;	        /*!< Patch version number */
    uint16_t iVersionBuild;	        /*!< Build number */
};

#endif // ADI_WIL_VERSION__H
