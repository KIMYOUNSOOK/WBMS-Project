/******************************************************************************
 * @file     adi_wil_script_change.h
 *
 * @brief    WBMS modify script data definitions
 *
 * @details  Contains modify script type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_SCRIPT_CHANGE__H
#define ADI_WIL_SCRIPT_CHANGE__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief Script change data structure
 *
 * @details The iActivationTime field specifies the time in the future at which the
 *          modified script entry is to be switched in. This field is in the same
 *          resolution (2ms) as the 3-byte timestamp field in each BMS/PMS data packet.
 *          The user can use the timestamp from BMS/PMS data packet as a reference to
 *          calculate an activation time: iActivationTime = current timestamp + offset.
 *          A maximum offset of 32767 can be used to give an equivalent of 65 seconds.
 *          Only the bottom 16-bits of the activation time is needed for iActivationTime.
 */
struct adi_wil_script_change_t {
    uint16_t iActivationTime;                                       /*!< Least significant 2 bytes of target timestamp when cell balancing commands are to be switched in */
    uint16_t iEntryOffset;                                          /*!< Byte offset of the script entry to change (offset of data bytes) */
    uint8_t  iChangeDataLength;                                     /*!< Length of change data  = Length of change data per monitor (8 bytes) * Number of monitors */
    uint8_t  iChangeData[ADI_WIL_MAX_SCRIPT_CHANGE_DATA_LENGTH];    /*!< Buffer containing script change data */
};

#endif //ADI_WIL_SCRIPT_CHANGE__H
