/******************************************************************************
 * @file     adi_wil_network_data.h
 *
 * @brief    WBMS network data definitions
 *
 * @details  Contains network data type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_NETWORK_DATA_H
#define ADI_WIL_NETWORK_DATA_H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief Radio related metadata
 */
struct adi_wil_network_data_t {
    adi_wil_device_t eSrcDeviceId;		        /*!< Packet source device ID */
    adi_wil_device_t eSrcManagerId;		        /*!< Device ID of the manager that the packet came through */
    uint64_t iPacketGenerationTime;             /*!< Network time at which the packet was generated on the source end */
    uint16_t iLatency;                          /*!< Packet Latency */
    uint32_t iSequenceNumber;                   /*!< Packet sequence number */
    uint8_t iChannel;                           /*!< Radio channel the packet used */
    int8_t iRSSI;                               /*!< Channel RSSI */
    uint8_t iReserved;                          /*!< Reserved */
};

#endif // ADI_WIL_NETWORK_DATA_H
