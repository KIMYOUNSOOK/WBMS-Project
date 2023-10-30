/******************************************************************************
 * @file     adi_wil_cell_balancing_status.h
 *
 * @brief    Cell balancing status structure definitions
 *
 * @details  Contains cell balancing status type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_CELL_BALANCING_STATUS_H
#define ADI_WIL_CELL_BALANCING_STATUS_H

#include "adi_wil_types.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief   Cell balancing status structure
 */
struct adi_wil_cell_balancing_status_t {
    adi_wil_device_t eDeviceId;             /*!< Device that sent this status message */
    uint64_t iEnabledCells;                 /*!< 64-bit map of cells which are currently being balanced. Bit 0 represents cell 0 and bit 63 represents cell 63 */
    uint16_t iRemainingDuration;            /*!< Duration (in minutes) remaining of the current cell balancing activity on this node */
    bool bThermalShutdown;                  /*!< Boolean indicating if a thermal shutdown is currently active on a BMIC connected to this node */
};

#endif //ADI_WIL_CELL_BALANCING_STATUS_H
