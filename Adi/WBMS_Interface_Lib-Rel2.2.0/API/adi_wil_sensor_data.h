/******************************************************************************
 * @file     adi_wil_sensor_data.h
 *
 * @brief    WBMS sensor data definitions
 *
 * @details  Contains sensor data type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_SENSOR_DATA_H
#define ADI_WIL_SENSOR_DATA_H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Sensor data structure
 */
struct adi_wil_sensor_data_t
{
    adi_wil_device_t eDeviceId;                         /*!< Device that produced this packet */
    uint16_t iLength;                                   /*!< Length of the data contained in the buffer */
    uint8_t Data[ADI_WIL_SENSOR_DATA_SIZE];             /*!< Buffer containing the payload data */
};

#endif // ADI_WIL_SENSOR_DATA_H
