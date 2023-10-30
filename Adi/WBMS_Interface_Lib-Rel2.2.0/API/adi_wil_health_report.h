/******************************************************************************
 * @file     adi_wil_health_report.h
 *
 * @brief    WBMS health report type definitions
 *
 * @details  Contains health report data type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_HEALTH_REPORT__H
#define ADI_WIL_HEALTH_REPORT__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief Health Report data structure
 */
struct adi_wil_health_report_t {
    adi_wil_device_t eDeviceId;                                 /*!< Device ID of the device that this report originated from */
    uint64_t iPacketGenerationTime;                             /*!< Packet generation time in ASN */
    uint8_t iLength;                                            /*!< Number of valid bytes in the buffer Data */
    uint8_t Data[ADI_WIL_HR_DATA_SIZE];                         /*!< Health report data */
};

#endif //ADI_WIL_HEALTH_REPORT__H
