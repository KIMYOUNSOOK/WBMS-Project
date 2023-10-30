/******************************************************************************
* @file    adi_wil_xms_parameters.h
*
* @brief   Definition of adi_wil_xms_parameters_t structure
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef ADI_XMS_PARAMETERS__H
#define ADI_XMS_PARAMETERS__H

#include <stdint.h>

/******************************************************************************
 * Typedefs
 *****************************************************************************/

/**
 * @brief   XMS measurements parameters
 */
typedef struct {
    uint64_t iBMSDevices;    /*!< Provides a bitmap of nodes that have indicated they will produce BMS data */
    uint64_t iPMSDevices;    /*!< Provides a bitmap of nodes that have indicated they will produce PMS data */
    uint64_t iEMSDevices;    /*!< Provides a bitmap of nodes that have indicated they will produce EMS data */
    uint8_t iBMSPackets;     /*!< Provides the number of packets to allocate for each BMS enabled node */
    uint8_t iPMSPackets;     /*!< Provides the number of packets to allocate for each EMS enabled node */
    uint8_t iEMSPackets;     /*!< Provides the number of packets to allocate for each PMS enabled node */
} adi_wil_xms_parameters_t;

#endif  //ADI_XMS_PARAMETERS__H
