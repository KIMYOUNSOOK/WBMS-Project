/******************************************************************************
 * @file     adi_wil_device_removed.h
 *
 * @brief    WBMS device removed definitions
 *
 * @details  Contains device removed event type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_DEVICE_REMOVED__H
#define ADI_WIL_DEVICE_REMOVED__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Device removal info
 */
struct adi_wil_device_removed_t {
    adi_wil_device_t eDeviceId;     /*!< Device Id of removed device */
    adi_wil_err_t rc;               /*!< Reason for device removal from transfer */
};

#endif //ADI_WIL_DEVICE_REMOVED__H
