/******************************************************************************
 * @file     adi_wil_dev_version.h
 *
 * @brief    Device firmware information type definition
 *
 * @details  Contains device firmware version data type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_DEV_VERSION__H
#define ADI_WIL_DEV_VERSION__H

#include "adi_wil_types.h"
#include "adi_wil_version.h"
#include <stdint.h>

/**
 * @brief   Device firmware information structure
 */
struct adi_wil_dev_version_t {
    adi_wil_version_t MainProcSWVersion;     /*!< Main processor software version number */
    uint16_t iMainProcSiVersion;             /*!< Main processor Silicon version number */
    adi_wil_version_t CoProcSWVersion;       /*!< Co-processor software version number */
    uint16_t iCoProcSiVersion;               /*!< Co-processor Silicon version number */
    uint32_t iLifeCycleInfo;                 /*!< Life cycle information */
};

#endif // ADI_WIL_DEV_VERSION__H
