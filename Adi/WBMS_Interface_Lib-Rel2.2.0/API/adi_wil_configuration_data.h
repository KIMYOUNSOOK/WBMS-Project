/******************************************************************************
 * @file     adi_wil_configuration_data.h
 *
 * @brief    WBMS network configuration definitions
 *
 * @details  Contains network configuration type definitions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_CONFIGURATION_DATA_H
#define ADI_WIL_CONFIGURATION_DATA_H

#include "adi_wil_types.h"
#include "adi_wil_version.h"
#include <stdint.h>

/**
 * @brief   QueryDevice response structure
 */
struct adi_wil_configuration_t {
    uint8_t MAC[ADI_WIL_MAC_ADDR_SIZE];         /*!< MAC of manager */
    uint8_t PeerMAC[ADI_WIL_MAC_ADDR_SIZE];     /*!< MAC of manager connected via M2M link */
    uint16_t iSensorPacketCount;                /*!< Size of adi_wil_sensor_data_t buffer required for this network */
    uint32_t iConfigurationHash;                /*!< Configuration hash */
    bool bDualConfiguration;                    /*!< Boolean indicating if the manager is configured in single or dual mode */
    bool bEncryptionEnabled;                    /*!< Boolean indicating if this device has encryption enabled */
    uint8_t Nonce[ADI_WIL_NONCE_SIZE];          /*!< Current nonce of the device at this port if encryption has been enabled */
    adi_wil_version_t MainProcSWVersion;        /*!< Main processor software version number */
};

#endif // ADI_WIL_CONFIGURATION_DATA_H
