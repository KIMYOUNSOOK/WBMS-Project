/******************************************************************************
 * @file     adi_wil_faults.h
 *
 * @brief    WBMS file data definitions
 *
 * @details  Contains file type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_FAULTS__H
#define ADI_WIL_FAULTS__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Fault information structure
 */
struct adi_wil_fault_report_t {
    adi_wil_device_t eDeviceId;                      /*!< Device that produced this fault report */
    uint16_t iAlertTypes;                            /*!< Bitmap representing alert types detected on this device */
    uint8_t iChannels[ADI_WIL_FAULT_CHANNELS_SIZE];  /*!< Bitmap representing channels on which plausibility faults have been detected */
};
#endif //ADI_WIL_FAULTS__H
