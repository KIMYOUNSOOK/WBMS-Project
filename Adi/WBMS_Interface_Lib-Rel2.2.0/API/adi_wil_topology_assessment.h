/******************************************************************************
 * @file     adi_wil_topology_assessment.h
 *
 * @brief    WBMS topology assessment type definitions
 *
 * @details  Contains topology assessment data type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_TOPOLOGY_ASSESSMENT__H
#define ADI_WIL_TOPOLOGY_ASSESSMENT__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief Topology assessment result data structure
 */
struct adi_wil_topology_assessment_t {
    int8_t iRssiDeltas [ADI_WIL_MAX_NODES];     /*!< Expected change in RSSI for all assessed nodes expressed in dBs */
    int8_t iSignalFloorImprovement;             /*!< Interference resilience improvement (worst path) expressed in dBs */
    adi_wil_err_t rc;                           /*!< Result of assessment process */
};

#endif //ADI_WIL_TOPOLOGY_ASSESSMENT__H
