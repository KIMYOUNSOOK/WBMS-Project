/******************************************************************************
 * @file     adi_wil_pack.h
 *
 * @brief    WBMS pack structure definitions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_PACK__H
#define ADI_WIL_PACK__H

#include "adi_wil_pack_internals.h"
#include "adi_wil_safety_internals.h"

/******************************************************************************
 * Structure Definitions
 *****************************************************************************/

/**
 * @brief   Pack structure
 */
struct adi_wil_pack_t
{
    adi_wil_pack_internals_t * pInternals;           /*!< Internals storage of pack instance */
    adi_wil_safety_internals_t * pSafetyInternals;   /*!< FuSa Internals storage of pack instance */
    adi_wil_port_t * pManager0Port;                  /*!< Manager 0 Port instance pointer */
    adi_wil_port_t * pManager1Port;                  /*!< Manager 1 Port instance pointer */
    adi_wil_sensor_data_t * pDataBuffer;             /*!< Sensor data storage pointer */
    void * pClientData;                              /*!< Client data pointer */
    uint16_t iDataBufferCount;                       /*!< Number of adi_wil_sensor_data_t elements in pDataBuffer array */
};

#endif //ADI_WIL_PACK__H
