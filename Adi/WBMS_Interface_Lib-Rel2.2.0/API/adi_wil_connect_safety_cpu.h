/******************************************************************************
 * @file     adi_wil_connect_safety_cpu.h
 *
 * @brief    WILFE Safety CPU connect message struct definition
 *
 * @details  Contains data type definition for connect message from Safety CPU
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_CONNECT_SAFETY_CPU_H
#define ADI_WIL_CONNECT_SAFETY_CPU_H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Connect message received from Safety CPU
 */
struct adi_wil_connect_safety_cpu_t {
    adi_wil_device_t eDeviceId;                          /*!< Device that sent this connect response */
    uint8_t NodeMAC [ADI_WIL_MAC_ADDR_SIZE];             /*!< MAC address of the wireless tail connected to the Safety CPU */
    uint8_t SafetyCPUSerialId [ADI_WIL_MAC_ADDR_SIZE];   /*!< MAC address or unique identifier of the Safety CPU */
    uint16_t iSoftwareVersionMajor;                      /*!< Software version Major of the Safety CPU */
    uint16_t iSoftwareVersionMinor;                      /*!< Software version Minor of the Safety CPU */
    uint16_t iSoftwareVersionPatch;                      /*!< Software version Patch of the Safety CPU */
    uint16_t iSoftwareVersionBuild;                      /*!< Software version Build of the Safety CPU */
    uint32_t iConfigurationCRC;                          /*!< Configuration CRC value */
    uint32_t iPMSContainerCRC;                           /*!< PMS Container CRC value */
    uint32_t iBMSContainerCRC;                           /*!< BMS Container CRC value */
    uint32_t iEMSContainerCRC;                           /*!< EMS Container CRC value */
    uint16_t iPMSInterval;                               /*!< PMS measurement interval */
    uint16_t iBMSInterval;                               /*!< BMS measurement interval */
    uint16_t iEMSInterval;                               /*!< EMS measurement interval */
    uint8_t iMaxPMSPackets;                              /*!< Maximum number of PMS packets in one measurement interval */
    uint8_t iMaxBMSPackets;                              /*!< Maximum number of BMS packets in one measurement interval */
    uint8_t iMaxEMSPackets;                              /*!< Maximum number of EMS packets in one measurement interval */
    uint8_t iOperatingMode;                              /*!< Operating mode of the Safety CPU */
    adi_wil_err_t rc;                                          /*!< Return code indicating success or failure of the connect request */
};

#endif //ADI_WIL_CONNECT_SAFETY_CPU_H
