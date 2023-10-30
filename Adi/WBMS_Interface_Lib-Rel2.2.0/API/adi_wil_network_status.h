/******************************************************************************
 * @file     adi_wil_network_status.h
 *
 * @brief    WBMS network status definitions
 *
 * @details  Contains network status type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_NETWORK_STATUS_H
#define ADI_WIL_NETWORK_STATUS_H

#include "adi_wil_types.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief   Network status structure
 */
struct adi_wil_network_status_t {
    adi_wil_device_t iConnectState;         /*!< A bitmask indicating which nodes are connected to the network, 1 indicates connected, 0 otherwise */
    uint8_t iCount;                         /*!< Number of nodes in the system */
};

#endif //ADI_WIL_NETWORK_STATUS_H
