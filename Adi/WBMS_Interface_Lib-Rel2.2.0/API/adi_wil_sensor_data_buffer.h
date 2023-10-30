/******************************************************************************
 * @file     adi_wil_sensor_data_buffer.h
 *
 * @brief    WBMS sensor data buffer definitions
 *
 * @details  Contains sensor data buffer type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_SENSOR_DATA_BUFFER_H
#define ADI_WIL_SENSOR_DATA_BUFFER_H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Sensor data buffer
 */
struct adi_wil_sensor_data_buffer_t {
    adi_wil_sensor_data_t * pData;                     /*!< Pointer to sensor data array */
    uint16_t iCount;                                   /*!< Number of entries in the array */
};

#endif // ADI_WIL_SENSOR_DATA_BUFFER_H
